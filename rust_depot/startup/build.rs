fn main() {
    println!("cargo:rerun-if-changed=src/load_modules.c");
    cc::Build::new()
        .file("src/load_modules.c")
        .include("../../include")
        .compile("load");
}
