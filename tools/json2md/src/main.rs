use std::env;
use std::fs::{self, File};
use std::io::Write;
use std::sync::{Once, OnceLock};
use anyhow::Result;
use log::{debug, info};

static MAX_LEVEL: OnceLock<i32> = OnceLock::new();

fn main() -> Result<()> {
    env_logger::init();

    let args: Vec<_> = env::args().collect();
    if args.len() < 3 {
        panic!("Usage: json2md <profile> <target> [maxlevel]");
    }
    let profile_name = &args[1];
    let target_name = &args[2];
    if args.len() == 4 {
        let _ = MAX_LEVEL.set((&args[3]).parse::<i32>()?);
    }
    debug!("Convert {} (root = {}) to mermaid.md maxlevel[{}] ...",
        profile_name, target_name, MAX_LEVEL.get().unwrap_or(&i32::MAX));

    let profile = fs::read_to_string(profile_name)?;
    let mut root = json::parse(&profile)?;
    assert!(root.has_key("dependencies"));
    let root = &mut root["dependencies"];
    assert!(root.has_key(target_name));

    let mut md = create_markdown(target_name)?;
    let mut level = 0;
    output_markdown(&mut md, root, target_name, &mut level)?;
    assert_eq!(level, 0);
    complete_markdown(md)?;

    Ok(())
}

fn output_markdown(
    f: &mut File, root: &mut json::JsonValue, name: &str, level: &mut i32,
) -> Result<()> {
    debug!("Current loglevel: {}", level);
    if let Some(maxlevel) = MAX_LEVEL.get() {
        if *level >= *maxlevel {
            static FLAG: Once = Once::new();
            FLAG.call_once(|| {
                info!("Reach max level, exit ahead of time!");
            });
            return Ok(());
        }
    }

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
        *level += 1;
        output_markdown(f, root, child.as_str().unwrap(), level)?;
        *level -= 1;
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
