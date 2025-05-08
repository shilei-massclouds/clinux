//! A tool to convert component-dependencies description file
//! from json-form to dot-form.
//!
//! For component-dependencies description, there's a toolchain:
//! 1) find_dep: dump dependencies in json form during building process.
//!     E.g., `make TOP=linux` to get 'target/_bootrd/top_linux.json'.
//! 2) json2dot(this tool): convert json to dot form.
//!     E.g., `json2dot top_linux.json top_linux 3` to get 'top_linux.gv'.
//! 3) graphviz tool: convert dot to diagram form.
//!     E.g., `dot -Tpng top_linux.gv -o top_linux.png`.

use std::env;
use std::fs::{self, File};
use std::io::Write;
use std::sync::Once;
use anyhow::{Result, anyhow};
use log::debug;

fn main() -> Result<()> {
    env_logger::init();

    let args: Vec<_> = env::args().collect();
    if args.len() != 3 && args.len() != 4 {
        println!("json2dot <json-filename> <root-nodename> [maxlevel]");
        println!(" - json-filename: dependencies description file in json.");
        println!("   E.g., 'target/_bootrd/top_linux.json'");
        println!(" - root-nodename: root node of diagram.");
        println!("   Any node that appears in json.");
        println!("   E.g., top_linux, linux, rbtree, ...");
        println!(" - maxlevel: Limit max level in diagram.");
        println!("   There's no limit when it is NOT specified.");
        return Err(anyhow!("Please give proper arguments!"));
    }
    let json_fname = &args[1];
    let root = &args[2];
    let maxlevel = if args.len() == 4 {
        args[3].parse::<i32>().ok()
    } else {
        None
    };
    debug!("Convert root='{}' in [{}]) to dot-form with maxlevel({:?}).",
        json_fname, root, maxlevel);

    let json_obj = fs::read_to_string(json_fname)?;
    let mut json_dep = json::parse(&json_obj)?;
    assert!(json_dep.has_key("dependencies"));
    let json_dep = &mut json_dep["dependencies"];
    assert!(json_dep.has_key(root));

    let mut dot = create_dot(root)?;
    let mut level = 0;
    output_dot(&mut dot, json_dep, root, &mut level, maxlevel)?;
    assert_eq!(level, 0);
    complete_dot(dot)?;
    Ok(())
}

fn create_dot(fname: &str) -> Result<File> {
    let fname = format!("{}.dot", fname);
    debug!("fname: {}", fname);
    let mut f = File::create(&fname)?;
    writeln!(f, "digraph DOT {{")?;
    Ok(f)
}

fn output_dot(
    f: &mut File,
    json_dep: &mut json::JsonValue,
    name: &str,
    level: &mut i32,
    maxlevel: Option<i32>,
) -> Result<()> {
    if let Some(max) = maxlevel {
        if *level >= max {
            static FLAG: Once = Once::new();
            FLAG.call_once(|| {
                debug!("Reach max level, exit ahead of time!");
            });
            return Ok(());
        }
    }

    if !json_dep.has_key(name) {
        return Ok(());
    }

    let target = json_dep.remove(name);
    assert!(target.is_array());
    if target.members().len() == 0 {
        return Ok(());
    }

    for child in target.members() {
        // ignore 'lds'
        if child == "lds" {
            continue;
        }
        debug!("child: {}", child);
        writeln!(f, "{}->{}", name, child)?;
        *level += 1;
        output_dot(f, json_dep, child.as_str().unwrap(), level, maxlevel)?;
        *level -= 1;
    }
    Ok(())
}

fn complete_dot(mut f: File) -> Result<()> {
    writeln!(f, "}}")?;
    Ok(())
}
