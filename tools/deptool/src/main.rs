use std::env;
use std::fs::File;
use std::io::Read;
use std::io::{BufReader, BufRead};
use std::collections::HashMap;
use std::sync::Arc;
use std::sync::atomic::Ordering;
use std::fs::OpenOptions;
use std::io::Write;
use std::path::Path;
use anyhow::{Result, anyhow};
use xmas_elf::ElfFile;
use xmas_elf::sections::SectionData::SymbolTable64;
use xmas_elf::symbol_table::Entry;
use xmas_elf::symbol_table::Type;
use xmas_elf::sections::SHN_UNDEF;

mod module;

use module::{ModuleStatus, Module, ModuleRef};

static BLACK_LIST: [&str; 5] = [
    "vmlinux",
    ".vmlinux.export",
    "scripts/dtc/",
    "scripts/kconfig/",
    "scripts/mod/",
];

static SAMPLE_LIST: [&str; 8] = [
    "mm/slab_common",
    "mm/page_alloc",
    "kernel/sched/core",
    "lib/bitmap",
    "block/blk-core",
    "net/socket",
    "fs/ext2/inode",
    "drivers/block/virtio_blk",
];

fn main() -> Result<()> {
    env_logger::init();

    let args: Vec<_> = env::args().collect();
    if args.len() != 3 {
        panic!("Missing <root_dir> <ext_name(.o|.ko)> !");
    }
    let kmod_path = &args[1];
    if !kmod_path.ends_with("/") {
        panic!("kmod_path must be suffixed with '/'.");
    }
    let ext_name = &args[2];
    if ext_name != ".o" && ext_name != ".ko" {
        panic!("ext_name must be .o or .ko");
    }
    log::debug!("Root path: '{}'; ext name: '{}'.", kmod_path, ext_name);

    let (sym_map, kmods) = discover_modules(&kmod_path, ext_name)?;
    for m in &kmods {
        build_dependency(&m, &sym_map)?;
    }

    log::debug!("traverse ...");
    // Note: Don't do Loop-Fusion.
    // We need a full loop process to build dependencies for all mods.
    let mut sum_deplen = 0;
    for m in &kmods {
        let deplen = traverse(m.clone(), 0)?;
        log::debug!("domain: {}, MaxDepChainLength: {:?}", m.name, deplen);
        sum_deplen += deplen;
    }

    let mut total_elements = 0;
    let mut sum_depwide = 0;
    for m in &kmods {
        log::debug!("domain: {}, DepDirectWide: {}", m.name, m.dependencies.borrow().len());
        sum_depwide += m.dependencies.borrow().len();
        total_elements += m.nr_elements.get().unwrap_or(&0);
    }

    let total_domains = kmods.len();
    let indicator_in_domain = total_elements as f64 / total_domains as f64;
    let adw = sum_depwide as f64 / total_domains as f64;
    let adl = sum_deplen as f64 / total_domains as f64;
    let di = (total_elements as f64 * adw * adl) / total_domains.pow(2) as f64;

    println!("******************* [Global] ********************");
    println!("Root path of target kernel:\t'{}'", kmod_path);
    println!("Total number of domains (#D):\t{}", total_domains);
    println!("Total number of elements (#E):\t{}", total_elements);
    println!("Average elements per domain:\t{:.2}", indicator_in_domain);
    println!("Average dependency wide (ADW):\t{:.2}", adw);
    println!("Average dependency len (ADL):\t{:.2}", adl);
    println!("Diffusion Indicator (DI):\t{:.2}", di);
    println!("");
    println!("Formula: DI = (#E / #D) * (ADW / #D) * ADL");
    println!("*********************************************\n");

    let version = get_last(kmod_path);
    dump_table("global", &version,
        total_domains, total_elements, indicator_in_domain,
        adw, adl, di)?;

    for m in &kmods {
        for sample in SAMPLE_LIST {
            if m.name == sample {
                let mut node_map = HashMap::<String, usize>::new();
                traverse_simple(m.clone(), &mut node_map)?;
                for node in &node_map {
                    log::debug!("{}: {}", node.0, node.1);
                }

                let nr_domains = node_map.len();
                let nr_elements = node_map.values().sum::<usize>();
                let indicator_in_domain = nr_elements as f64 / nr_domains as f64;
                let dw = m.dependencies.borrow().len() as f64;
                let dl = m.max_deplen.load(Ordering::Relaxed) as f64;
                let di = (nr_elements as f64 * dw * dl) / (nr_domains as f64 * total_domains as f64);
                println!("******************* [{}] ********************", sample);
                println!("Number of domains (#D):\t\t{}", nr_domains);
                println!("Number of elements (#E):\t{}", nr_elements);
                println!("Average elements per domain:\t{:.2}", indicator_in_domain);
                println!("Dependency wide (DW):\t\t{:.2}", dw);
                println!("Max dependency length (DL):\t{:.2}", dl);
                println!("Diffusion Indicator (DI):\t{:.2}", di);
                println!("");
                println!("Formula: DI = (#E / #D) * (DW / #TD) * DL");
                println!("*********************************************\n");

                dump_table(sample, &version,
                    nr_domains, nr_elements, indicator_in_domain,
                    dw, dl, di)?;
            }
        }
    }
    Ok(())
}

fn dump_table(
    sample: &str,
    version: &str,
    domains: usize,
    elements: usize,
    i_in_d: f64,
    adw: f64,
    adl: f64,
    di: f64
) -> Result<()> {
    let sample = get_last(sample);
    let fname = format!("/tmp/{}", sample);
    if !Path::new(&fname).exists() {
        File::create(&fname)?;
    }

    let mut f = OpenOptions::new().append(true).open(fname)?;
    writeln!(f, "|{}|{}|{}|{:.2}|{:.2}|{:.2}|{:.2}|",
             version, domains, elements, i_in_d, adw, adl, di)?;
    Ok(())
}

fn get_last(path: &str) -> String {
    let path = path.trim_end_matches('/');
    let parts: Vec<_> = path.split('/').collect();
    let count = parts.len();
    assert!(count > 0);
    parts[count - 1].to_owned()
}

// This fuction is just used to discover all modules in diffusion sector.
// It's hard to reuse 'traverse' for me. Maybe someone can do it in future.
// Note: in traverse_simple, we ignore module's status.
fn traverse_simple(
    kmod: ModuleRef,
    node_map: &mut HashMap::<String, usize>
) -> Result<()> {
    log::debug!("mod name: {}", kmod.name);
    if node_map.get(&kmod.name).is_some() {
        return Ok(());
    }
    node_map.insert(kmod.name.clone(), *kmod.nr_elements.get().unwrap());

    for depend in kmod.dependencies.borrow().iter() {
        traverse_simple(depend.clone(), node_map)?;
    }
    Ok(())
}

fn traverse(kmod: ModuleRef, level: usize) -> Result<usize> {
    let status = kmod.status.fetch_or(
        ModuleStatus::TOUCHED.bits(),
        Ordering::Relaxed
    );
    let status = ModuleStatus::from_bits_truncate(status);
    if status.contains(ModuleStatus::TOUCHED) {
        if status.contains(ModuleStatus::DONE) {
            let max_deplen = kmod.max_deplen.load(Ordering::Relaxed);
            return Ok(max_deplen);
        } else {
            log::debug!("Find cyclic chain level: {}!", level);
            let max_deplen = level * 2;
            kmod.max_deplen.store(max_deplen, Ordering::Relaxed);
            return Ok(max_deplen);
        }
    }

    let mut max_deplen = 0;
    for depend in kmod.dependencies.borrow().iter() {
        let cur_deplen = traverse(depend.clone(), level + 1)?;
        if max_deplen < cur_deplen {
            max_deplen = cur_deplen;
            log::debug!("{} -> {}. max: {}.", kmod.name, depend.name, max_deplen);
        }
    }

    let mut ret_deplen = kmod.max_deplen.load(Ordering::Relaxed);
    if ret_deplen <= max_deplen {
        // Add current level to deplen.
        ret_deplen = max_deplen + 1;
        kmod.max_deplen.store(ret_deplen, Ordering::Relaxed);
    }
    kmod.status.fetch_or(ModuleStatus::DONE.bits(), Ordering::Relaxed);
    log::debug!("Check dependency chain for {} [{}].", kmod.name, ret_deplen);
    Ok(ret_deplen)
}

fn build_dependency(kmod: &ModuleRef, sym_map: &HashMap<String, ModuleRef>) -> Result<()> {
    let mut remained = Vec::<String>::new();
    let undef_syms = kmod.undef_syms.take();
    assert!(kmod.undef_syms.borrow().is_empty());
    for undef in undef_syms {
        if let Some(dep) = sym_map.get(&undef) {
            log::debug!("{} -> {}:{}", kmod.name, dep.name, undef);
            if !find_dependency(dep, &kmod.name) {
                dep.dependencies.borrow_mut().push(kmod.clone());
                build_dependency(dep, sym_map)?;
            }
        } else {
            remained.push(undef);
        }
    }

    if !remained.is_empty() {
        log::debug!("mod '{}' has undef symbols:\n{}\n",
            kmod.name, remained.join("\n"));
    }
    Ok(())
}

fn find_dependency(kmod: &ModuleRef, name: &str) -> bool {
    kmod.dependencies.borrow().iter().find(|x| x.name == name).is_some()
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
                    log::debug!("domain '{}': undef symbol: {}", kmod.name, name);
                    kmod.undef_syms.borrow_mut().push(name.to_owned());
                }
            }
        }
    } else {
        log::warn!("Domain '{}': bad section .symtab.", kmod.name);
    }
    Ok(())
}

fn discover_modules(
    kmod_path: &str,
    ext_name: &str,
) -> Result<(HashMap<String, ModuleRef>, Vec::<ModuleRef>)> {
    let mut sym_map: HashMap<String, ModuleRef> = HashMap::new();
    let mut modules = Vec::<ModuleRef>::new();

    let expr = format!("{}/**/*{}", kmod_path, ext_name);
    for entry in glob::glob(expr.as_str())? {
        let entry = entry?;
        let path = entry.to_str()
            .ok_or(anyhow!("bad entry: {:?}", entry))?;
        let name = path.strip_prefix(kmod_path)
            .ok_or(anyhow!("bad module path: '{}'", path))?;
        let name = name.strip_suffix(ext_name)
            .ok_or(anyhow!("bad module name: '{}'", name))?;
        log::debug!("path: {}; name: {}", path, name);

        if be_in_black_list(name) {
            continue;
        }

        let mut kmod = Module::new(name);

        let buf = load_module(path)?;
        log::debug!("buf.len: {}", buf.len());
        let elf = ElfFile::new(&buf).expect("bad elf!");
        detect_undef_symbols(&mut kmod, &elf)?;

        let kmod = Arc::new(kmod);
        export_symbols(&mut sym_map, &kmod, &elf)?;
        modules.push(kmod.clone());
        log::debug!("domain: {} ok!", kmod.name);
    }

    add_lds_symbols(&mut sym_map)?;
    log::debug!("discover ok!");
    Ok((sym_map, modules))
}

fn be_in_black_list(name: &str) -> bool {
    for item in BLACK_LIST {
        if item.ends_with('/') {
            if name.starts_with(item) {
                return true;
            }
        } else {
            if name == item {
                return true;
            }
        }
    }
    false
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
        log::debug!("line: {}", line);
        sym_map.insert(line.to_owned(), kmod.clone());
    }
    Ok(())
}

fn export_symbols(
    sym_map: &mut HashMap<String, ModuleRef>,
    kmod: &ModuleRef,
    elf: &ElfFile
) -> Result<()> {
    if let Some(sec) = elf.find_section_by_name(".symtab") {
        let data = sec.get_data(&elf).unwrap();
        if let SymbolTable64(symbols) = data {
            let mut nr_elements = 0;
            for symbol in symbols {
                if symbol.shndx() == SHN_UNDEF {
                    continue;
                }

                let ty = symbol.get_type().unwrap();
                if matches!(ty, Type::Func | Type::Object) {
                    let name = symbol.get_name(&elf).unwrap();
                    sym_map.insert(name.to_owned(), kmod.clone());
                    nr_elements += 1;
                }
            }
            kmod.nr_elements.set(nr_elements).unwrap();
            log::debug!("Domain '{}' has {} elements.", kmod.name, nr_elements);
        }
    }
    Ok(())
}

fn load_module(path: &str) -> Result<Vec<u8>> {
    let mut buf = vec![];
    let mut f = File::open(path)?;
    f.read_to_end(&mut buf)?;
    Ok(buf)
}
