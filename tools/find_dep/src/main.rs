use std::env;
use std::fs::File;
use std::io::Read;
use std::io::{BufReader, BufRead};
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use std::collections::HashMap;
use std::cell::RefCell;
use std::path::Path;
use xmas_elf::ElfFile;
use xmas_elf::sections::ShType::SymTab;
use xmas_elf::sections::SectionData::SymbolTable64;
use xmas_elf::symbol_table::Entry;
use xmas_elf::symbol_table::Entry64;
use xmas_elf::sections::SHN_UNDEF;
use anyhow::{Result, anyhow};

struct Module {
    name: String,
    done: AtomicBool,
    undef_syms: RefCell<Vec<String>>,
    dependencies: RefCell<Vec<ModuleRef>>,
}

impl Module {
    pub fn new(name: &str) -> Self {
        Self {
            name: name.to_owned(),
            done: AtomicBool::new(false),
            undef_syms: RefCell::new(vec![]),
            dependencies: RefCell::new(vec![]),
        }
    }
}

type ModuleRef = Arc<Module>;

fn main() -> Result<()> {
    let args: Vec<_> = env::args().collect();
    if args.len() != 3 {
        panic!("find_dep [kmod_dir] [top_name]");
    }
    let kmod_path = &args[1];
    let top_name = &args[2];
    println!("find_dep {} {}", kmod_path, top_name);

    let (top_kmod, sym_map) = discover_modules(&kmod_path, &top_name)?;
    build_dependency(&top_kmod, &sym_map)?;
    traverse(top_kmod)?;
    Ok(())
}

fn traverse(kmod: ModuleRef) -> Result<()> {
    if kmod.done.swap(true, Ordering::Relaxed) {
        return Ok(());
    }
    for depend in kmod.dependencies.borrow().iter() {
        traverse(depend.clone())?;
        println!("{} -> {}", kmod.name, depend.name);
    }
    println!("Seq: {}", kmod.name);
    Ok(())
}

fn find_dependency(kmod: &ModuleRef, name: &str) -> bool {
    kmod.dependencies.borrow().iter().find(|x| x.name == name).is_some()
}

fn build_dependency(kmod: &ModuleRef, sym_map: &HashMap<String, ModuleRef>) -> Result<()> {
    let mut remained = Vec::<String>::new();
    while let Some(undef) = kmod.undef_syms.borrow_mut().pop() {
        if let Some(dep) = sym_map.get(&undef) {
            if !find_dependency(kmod, &dep.name) {
                println!("undef: {} {}", undef, dep.name);
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
        println!("path: {}; name: {}", path, name);

        let mut kmod = Module::new(name);

        let buf = load_module(path)?;
        println!("buf.len: {}", buf.len());
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
    let mut kmod = Arc::new(Module::new("lds"));

    let cwd = env::current_exe().unwrap();
    let cwd = cwd.parent().unwrap().parent().unwrap().parent().unwrap();
    let conf = cwd.join("lds.conf");
    let f = BufReader::new(File::open(conf.clone())?);
    for line in f.lines().map_while(Result::ok) {
        let line = line.trim();
        if line.is_empty() {
            continue;
        }
        println!("line: {}", line);
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
        //println!("sec: {:?}", sec.get_data(&elf).unwrap().strings());
        println!("sec: {:?} {}", sec, sec.get_name(&elf).unwrap());
        let data = sec.raw_data(&elf);
        let data = std::str::from_utf8(data)?;
        for symbol in data.split("\0\0") {
            let symbol = symbol.trim();
            if !symbol.is_empty() {
                println!("export symbol: {}", symbol);
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
                    println!("symbol: {}", name);
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
