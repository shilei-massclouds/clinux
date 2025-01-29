use std::env;
use std::fs::File;
use std::io::Read;
use std::sync::Arc;
use std::sync::atomic::AtomicBool;
use std::collections::HashMap;
use std::cell::RefCell;
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

    let mods = discover_modules(&kmod_path, &top_name)?;
    Ok(())
}

fn discover_modules(kmod_path: &str, top_name: &str) -> Result<()> {
    let mut has_top = false;
    let mut sym_map: HashMap<String, ModuleRef> = HashMap::new();
    let modules = Vec::<ModuleRef>::new();

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
        if !has_top && name == top_name {
            has_top = true;
        }

        let mut kmod = Module::new(name);

        let buf = load_module(path)?;
        println!("buf.len: {}", buf.len());
        let elf = ElfFile::new(&buf).expect("bad elf!");
        detect_undef_symbols(&mut kmod, &elf)?;

        let kmod = Arc::new(kmod);
        export_symbols(&mut sym_map, &kmod, &elf)?;

        /*
        */
    }

    if !has_top {
        panic!("No top '{}'\n", top_name);
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
