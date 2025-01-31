use std::env;
use std::fs::File;
use std::io::{Read, Write};
use std::io::{BufReader, BufRead};
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};
use std::collections::HashMap;
use std::cell::RefCell;
use xmas_elf::ElfFile;
use xmas_elf::sections::SectionData::SymbolTable64;
use xmas_elf::symbol_table::Entry;
use xmas_elf::sections::SHN_UNDEF;
use anyhow::{Result, anyhow};
use bitflags::bitflags;
use log::debug;

bitflags! {
    /// Module Status (bitwise layout).
    struct ModuleStatus: usize {
        const INITIAL   = 0b00;
        const TOUCHED   = 0b01;
        const DONE      = 0b10;
    }
}

struct Module {
    name: String,
    status: AtomicUsize,
    undef_syms: RefCell<Vec<String>>,
    dependencies: RefCell<Vec<ModuleRef>>,
}

impl Module {
    pub fn new(name: &str) -> Self {
        Self {
            name: name.to_owned(),
            status: AtomicUsize::new(ModuleStatus::INITIAL.bits()),
            undef_syms: RefCell::new(vec![]),
            dependencies: RefCell::new(vec![]),
        }
    }
}

type ModuleRef = Arc<Module>;

struct Payload {
    names: Vec<String>,
    last_msg: String,
    json: json::JsonValue,
}

impl Payload {
    pub fn new() -> Self {
        Self {
            names: vec![],
            last_msg: String::new(),
            json: json::JsonValue::new_object(),
        }
    }
}

fn main() -> Result<()> {
    env_logger::init();

    let args: Vec<_> = env::args().collect();
    if args.len() != 3 {
        panic!("find_dep [kmod_dir] [top_name]");
    }
    let kmod_path = &args[1];
    let top_name = &args[2];
    debug!("find_dep {} {}", kmod_path, top_name);

    let (top_kmod, sym_map) = discover_modules(&kmod_path, &top_name)?;
    build_dependency(&top_kmod, &sym_map)?;

    let mut payload = Payload::new();
    traverse(top_kmod, &mut payload)?;
    assert_eq!(payload.names.remove(0), "lds");
    debug!("Selected: {}", payload.names.join(" "));
    output_components(&payload, kmod_path)?;
    assert_eq!(payload.names.remove(0), "booter");
    output_initfile(&payload, kmod_path)?;
    output_json(&payload, kmod_path, top_name)?;
    Ok(())
}

fn output_components(payload: &Payload, path: &str) -> Result<()> {
    let names: Vec<String> = payload.names.iter().map(|x| {
        format!("{}{}.ko", path, x)
    }).collect();
    let fname = format!("{}selected.in", path);
    let mut f = File::create(fname)?;
    f.write_all(names.join(" ").as_bytes())?;
    Ok(())
}

fn output_initfile(payload: &Payload, path: &str) -> Result<()> {
    let fname = format!("{}cl_init.c", path);
    let mut f = File::create(fname)?;
    for name in &payload.names {
        writeln!(f, "extern int cl_{}_init();", name)?;
    }
    writeln!(f, "")?;
    writeln!(f, "int cl_init()")?;
    writeln!(f, "{{")?;
    for name in &payload.names {
        writeln!(f, "    cl_{}_init();", name)?;
    }
    writeln!(f, "    return 0;")?;
    writeln!(f, "}}")?;
    Ok(())
}

fn output_json(payload: &Payload, path: &str, top: &str) -> Result<()> {
    let mut root = json::JsonValue::new_object();
    root["dependencies"] = payload.json.clone();
    let fname = format!("{}{}.json", path, top);
    let mut f = File::create(fname)?;
    writeln!(f, "{}", root.dump())?;
    Ok(())
}

fn traverse(kmod: ModuleRef, payload: &mut Payload) -> Result<()> {
    let status = kmod.status.fetch_or(
        ModuleStatus::TOUCHED.bits(),
        Ordering::SeqCst
    );
    let status = ModuleStatus::from_bits_truncate(status);
    if status.contains(ModuleStatus::TOUCHED) {
        if status.contains(ModuleStatus::DONE) {
            return Ok(());
        }
        panic!("Cyclic chain: {}", payload.last_msg);
    }

    let mut json_array = json::JsonValue::new_array();
    for depend in kmod.dependencies.borrow().iter() {
        payload.last_msg = format!("{} -> {}", kmod.name, depend.name);
        traverse(depend.clone(), payload)?;
        payload.last_msg = String::new();
        json_array.push(depend.name.clone())?;
        debug!("{} -> {}", kmod.name, depend.name);
    }
    kmod.status.fetch_or(ModuleStatus::DONE.bits(), Ordering::SeqCst);
    payload.names.push(kmod.name.clone());
    payload.json[kmod.name.clone()] = json_array;
    Ok(())
}

fn find_dependency(kmod: &ModuleRef, name: &str) -> bool {
    kmod.dependencies.borrow().iter().find(|x| x.name == name).is_some()
}

fn build_dependency(kmod: &ModuleRef, sym_map: &HashMap<String, ModuleRef>) -> Result<()> {
    let mut remained = Vec::<String>::new();
    let undef_syms = kmod.undef_syms.take();
    assert!(kmod.undef_syms.borrow().is_empty());
    for undef in undef_syms {
        if let Some(dep) = sym_map.get(&undef) {
            if !find_dependency(kmod, &dep.name) {
                debug!("undef: {} {}", undef, dep.name);
                kmod.dependencies.borrow_mut().push(dep.clone());
                build_dependency(dep, sym_map)?;
            }
        } else {
            remained.push(undef);
        }
    }

    if !remained.is_empty() {
        panic!("mod '{}' has undef symbols:\n{}\n",
            kmod.name, remained.join("\n"));
    }
    Ok(())
}

fn discover_modules(
    kmod_path: &str, top_name: &str
) -> Result<(ModuleRef, HashMap<String, ModuleRef>)> {
    let mut top_kmod = None;
    let mut sym_map: HashMap<String, ModuleRef> = HashMap::new();
    let mut modules = Vec::<ModuleRef>::new();

    let expr = format!("{}/*.ko", kmod_path);
    for entry in glob::glob(expr.as_str())? {
        let entry = entry?;
        let path = entry.to_str()
            .ok_or(anyhow!("bad entry: {:?}", entry))?;
        let name = path.strip_prefix(kmod_path)
            .ok_or(anyhow!("bad module path: '{}'", path))?;
        let name = name.strip_suffix(".ko")
            .ok_or(anyhow!("bad module name: '{}'", name))?;
        debug!("path: {}; name: {}", path, name);

        let mut kmod = Module::new(name);

        let buf = load_module(path)?;
        debug!("buf.len: {}", buf.len());
        let elf = ElfFile::new(&buf).expect("bad elf!");
        detect_undef_symbols(&mut kmod, &elf)?;

        let kmod = Arc::new(kmod);
        export_symbols(&mut sym_map, &kmod, &elf)?;
        modules.push(kmod.clone());

        if name == top_name {
            assert!(top_kmod.is_none());
            top_kmod = Some(kmod);
        }
    }

    if top_kmod.is_none() {
        panic!("No top '{}'\n", top_name);
    }
    add_lds_symbols(&mut sym_map)?;
    Ok((top_kmod.unwrap(), sym_map))
}

fn add_lds_symbols(sym_map: &mut HashMap<String, ModuleRef>) -> Result<()> {
    let kmod = Arc::new(Module::new("lds"));

    let cwd = env::current_exe().unwrap();
    let cwd = cwd.parent().unwrap().parent().unwrap().parent().unwrap();
    let conf = cwd.join("lds.conf");
    let f = BufReader::new(File::open(conf.clone())?);
    for line in f.lines().map_while(Result::ok) {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        debug!("line: {}", line);
        sym_map.insert(line.to_owned(), kmod.clone());
    }
    Ok(())
}

fn export_symbols(
    sym_map: &mut HashMap<String, ModuleRef>,
    kmod: &ModuleRef,
    elf: &ElfFile
) -> Result<()> {
    if let Some(sec) = elf.find_section_by_name("__ksymtab_strings") {
        debug!("sec: {:?} {}", sec, sec.get_name(&elf).unwrap());
        let data = sec.raw_data(&elf);
        let data = std::str::from_utf8(data)?;
        for symbol in data.split("\0\0") {
            let symbol = symbol.trim();
            if !symbol.is_empty() {
                debug!("export symbol: {}", symbol);
                sym_map.insert(symbol.to_owned(), kmod.clone());
            }
        }
    }
    Ok(())
}

fn detect_undef_symbols(kmod: &mut Module, elf: &ElfFile) -> Result<()> {
    let sec = elf.find_section_by_name(".symtab")
        .ok_or(anyhow!("cannot find .symtabl."))?;
    let data = sec.get_data(&elf).unwrap();
    if let SymbolTable64(symbols) = data {
        for symbol in symbols {
            if symbol.shndx() == SHN_UNDEF {
                let name = symbol.get_name(&elf).unwrap();
                let name = name.trim();
                if !name.is_empty() {
                    debug!("symbol: {}", name);
                    kmod.undef_syms.borrow_mut().push(name.to_owned());
                }
            }
        }
    } else {
        panic!("bad section .symtab.");
    }
    Ok(())
}

fn load_module(path: &str) -> Result<Vec<u8>> {
    let mut buf = vec![];
    let mut f = File::open(path)?;
    f.read_to_end(&mut buf)?;
    Ok(buf)
}
