use std::env;
use std::fs::{self, File};
use std::io::Write;
use anyhow::Result;
use log::debug;

fn main() -> Result<()> {
    env_logger::init();

    let args: Vec<_> = env::args().collect();
    if args.len() != 3 {
        panic!("Usage: json2md <profile> <target>");
    }
    let profile_name = &args[1];
    let target_name = &args[2];
    debug!("Convert {} (root = {}) to mermaid.md ...",
        profile_name, target_name);

    let profile = fs::read_to_string(profile_name)?;
    let mut root = json::parse(&profile)?;
    assert!(root.has_key("dependencies"));
    let root = &mut root["dependencies"];
    assert!(root.has_key(target_name));

    let mut md = create_markdown(target_name)?;
    output_markdown(&mut md, root, target_name)?;
    complete_markdown(md)?;

    Ok(())
}

fn output_markdown(
    f: &mut File, root: &mut json::JsonValue, name: &str
) -> Result<()> {
    if !root.has_key(name) {
        return Ok(());
    }

    let target = root.remove(name);
    assert!(target.is_array());
    if target.members().len() == 0 {
        return Ok(());
    }

    let value = target.members()
        .map(|item| item.as_str().unwrap())
        .collect::<Vec<_>>()
        .join(" & ");
    writeln!(f, "{} --> {}", name, value)?;

    for child in target.members() {
        //debug!("child: {}", child);
        output_markdown(f, root, child.as_str().unwrap())?;
    }

    Ok(())
}

fn create_markdown(fname: &str) -> Result<File> {
    let fname = format!("{}.md", fname);
    debug!("fname: {}", fname);
    let mut f = File::create(&fname)?;
    writeln!(f, "### Profile: {}", fname)?;
    writeln!(f, "```mermaid")?;
    writeln!(f, "graph TD")?;
    Ok(f)
}

fn complete_markdown(mut f: File) -> Result<()> {
    writeln!(f, "```")?;
    Ok(())
}
