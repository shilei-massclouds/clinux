// SPDX-License-Identifier: Apache-2.0

// Copyright (C) 2019  Frank Rehberger
// Copyright (C) 2017  Kevin Pansky

use std::fs::File;
use std::io::Read;
use pest::Parser;
use pest::iterators::Pairs;
use pest::iterators::Pair;
use string_template::Template;
use std::collections::HashMap;

#[macro_use]
extern crate pest;

#[macro_use]
extern crate pest_derive;

/// OMG IDL v4 parser
#[derive(Parser)]
#[grammar = "grammar/idl_v4.pest"]
pub struct IdlParser;

struct Param {
    name: String,
    ptype: String,
    is_sequence: bool,
    attr: String,
}

impl Param {
    fn new() -> Param {
        Param {
            name: String::new(),
            ptype: String::new(),
            is_sequence: false,
            attr: String::new(),
        }
    }
}

struct Method {
    name: String,
    rtype: String,
    params: Vec<Param>,
}

impl Method {
    fn new() -> Method {
        Method {
            name: String::new(),
            rtype: String::new(),
            params: Vec::<Param>::new(),
        }
    }
}

struct Interface {
    name: String,
    methods: Vec<Method>,
}

impl Interface {
    fn new() -> Interface {
        Interface {
            name: String::new(),
            methods: Vec::<Method>::new(),
        }
    }
}

struct Context {
    output_dir: String,
    option: String,
    interfaces: Vec<Interface>,
    cur_itf: Option<Interface>,
    cur_method: Option<Method>,
    cur_param: Option<Param>,
    scope_stack: Vec<Rule>,
}

impl Context {
    fn new() -> Context {
        Context {
            output_dir: String::new(),
            option: String::new(),
            interfaces: Vec::<Interface>::new(),
            cur_itf: None,
            cur_method: None,
            cur_param: None,
            scope_stack: Vec::<Rule>::new(),
        }
    }
}

fn type_idl_to_rust(itype: &str) -> &str {
    match itype {
        "string"    => "*const core::ffi::c_char",
        "char"      => "core::ffi::c_char",
        "octet"     => "core::ffi::c_uchar",
        "short"     => "core::ffi::c_short",
        "long"      => "core::ffi::c_int",
        "long long" => "core::ffi::c_long",
        "boolean"   => "bool",
        "unsigned short"     => "core::ffi::c_ushort",
        "unsigned long"      => "core::ffi::c_uint",
        "unsigned long long" => "core::ffi::c_ulong",
        _ => {
            panic!("no type '{}'", itype);
        }
    }
}

fn fe_process(pair: &Pair<Rule>, ctx: &mut Context) {
    let iter = pair.clone().into_inner();
    //println!("BEGIN: rust '{:?}': {}", pair.as_rule(), pair.as_str());
    match pair.as_rule() {
        Rule::interface_dcl => {
            assert!(ctx.scope_stack.is_empty());
            assert!(ctx.cur_itf.is_none());
            ctx.cur_itf = Some(Interface::new());
            ctx.scope_stack.push(pair.as_rule());
            for p in iter {
                fe_process(&p, ctx);
            }
            ctx.scope_stack.pop();
            ctx.interfaces.push(ctx.cur_itf.take().unwrap());
        },
        Rule::interface_kind => {
            // Now only support 'interface'
            assert!(pair.as_str() == "interface");
        },
        Rule::op_dcl => {
            assert!(!ctx.cur_itf.is_none());
            ctx.cur_method = Some(Method::new());
            ctx.scope_stack.push(pair.as_rule());
            for p in iter {
                fe_process(&p, ctx);
            }
            ctx.scope_stack.pop();
            if let Some(itf) = &mut (ctx.cur_itf) {
                itf.methods.push(ctx.cur_method.take().unwrap());
            } else {
                panic!("no interface!");
            }
        },
        Rule::op_type_spec => {
            if let Some(method) = &mut (ctx.cur_method) {
                assert!(method.rtype.is_empty());
                method.rtype = String::from(pair.as_str().trim());
            } else {
                panic!("no method!");
            }
        },
        Rule::param_dcl => {
            assert!(!ctx.cur_method.is_none());
            ctx.cur_param = Some(Param::new());
            ctx.scope_stack.push(pair.as_rule());
            for p in iter {
                fe_process(&p, ctx);
            }
            ctx.scope_stack.pop();
            if let Some(method) = &mut (ctx.cur_method) {
                method.params.push(ctx.cur_param.take().unwrap());
            } else {
                panic!("no method!");
            }
        },
        Rule::param_attribute => {
            if let Some(param) = &mut (ctx.cur_param) {
                assert!(param.attr.is_empty());
                param.attr = String::from(pair.as_str());
            } else {
                panic!("no param!");
            }
        },
        Rule::type_spec => {
            if let Some(param) = &mut (ctx.cur_param) {
                assert!(param.ptype.is_empty() ||
                        param.ptype.starts_with("sequence"),
                        "param type: {}", param.ptype);
                param.ptype = String::from(pair.as_str().trim());
                ctx.scope_stack.push(pair.as_rule());
                for p in iter {
                    fe_process(&p, ctx);
                }
                ctx.scope_stack.pop();
            } else {
                panic!("no param!");
            }
        },
        Rule::sequence_type => {
            if let Some(param) = &mut (ctx.cur_param) {
                param.is_sequence = true;
                ctx.scope_stack.push(pair.as_rule());
                for p in iter {
                    fe_process(&p, ctx);
                }
                ctx.scope_stack.pop();
            } else {
                panic!("no param!");
            }
        },
        Rule::identifier => {
            if let Some(scope) = ctx.scope_stack.last() {
                match scope {
                    Rule::interface_dcl => {
                        if let Some(itf) = &mut (ctx.cur_itf) {
                            assert!(itf.name.is_empty());
                            itf.name = String::from(pair.as_str());
                        } else {
                            panic!("no interface!");
                        }
                    },
                    Rule::op_dcl => {
                        if let Some(method) = &mut (ctx.cur_method) {
                            assert!(method.name.is_empty());
                            method.name = String::from(pair.as_str());
                        } else {
                            panic!("no method!");
                        }
                    },
                    Rule::param_dcl => {
                        if let Some(param) = &mut (ctx.cur_param) {
                            assert!(param.name.is_empty());
                            param.name = String::from(pair.as_str());
                        } else {
                            panic!("no param!");
                        }
                    },
                    _ => {
                        panic!("no implementation!");
                    }
                }
            }
        },
        _ => {
            for p in iter {
                fe_process(&p, ctx);
            }
        }
    }
    //println!("END: rust '{:?}'", pair.as_rule());
}

fn make_rust_param(param: &Param, file: &mut File) {
    use std::io::Write;

    let ptype = type_idl_to_rust(&(param.ptype));

    if param.is_sequence {
        write!(file, ", {}: *const {}", param.name, ptype).unwrap();
        write!(file, ", {}_size: core::ffi::c_uint", param.name).unwrap();
    } else {
        write!(file, ", {}: {}", param.name, ptype).unwrap();
    }
}

fn make_rust_method(method: &Method, file: &mut File) {
    use std::io::Write;

    write!(file, "\n    fn {}(&self", method.name).unwrap();

    for param in &(method.params) {
        make_rust_param(param, file);
    }

    write!(file, ")").unwrap();

    if method.rtype.is_empty() {
        write!(file, ";").unwrap();
    } else {
        write!(file, " -> {};", type_idl_to_rust(&(method.rtype)))
            .unwrap();
    }
}

fn make_rust_interface(itf: &Interface, output_dir: &str) {
    use std::io::Write;

    let path = format!("{}/{}.rs", output_dir, &(itf.name).to_lowercase());
    let mut file = File::create(path).unwrap();

    write!(file, "pub trait {name} {{", name = itf.name).unwrap();
    for method in &(itf.methods) {
        make_rust_method(method, &mut file);
    }
    write!(file, "\n}}\n").unwrap();
}

fn make_method_for_skeleton(method: &Method, itf_name: &str,
                            block: &mut String) {
    use std::fmt::Write;

    /* read template from file */
    let tpl = include_str!("./grammar/method_in_skeleton.tpl");
    let template = Template::new(tpl);

    let c_method = format!("{interface}_{method}",
                           interface = itf_name,
                           method = method.name);

    let rtype;
    if !method.rtype.is_empty() {
        rtype = format!(" -> {}", type_idl_to_rust(&(method.rtype)));
    } else {
        rtype = String::new();
    }

    let mut decl_args = String::new();
    let mut call_args = String::new();
    for param in &(method.params) {
        let ptype = type_idl_to_rust(&(param.ptype));
        if param.is_sequence {
            write!(decl_args,
                   ", {}: *const {}", param.name, ptype).unwrap();
            write!(decl_args,
                   ", {}_size: core::ffi::c_uint", param.name).unwrap();
        } else {
            write!(decl_args, "{}: {}, ", param.name, ptype).unwrap();
        }
        write!(call_args, "{}, ", param.name).unwrap();
    }

    let mut args = HashMap::<&str, &str>::new();
    args.insert("method", &(method.name));
    args.insert("c_method", &c_method);
    let c_method_len = c_method.len().to_string();
    args.insert("len", &c_method_len);
    args.insert("rtype", &rtype);
    args.insert("decl_args", &decl_args);
    args.insert("call_args", &call_args);

    write!(block, "{}\n", template.render(&args)).unwrap();
}

fn make_rust_skeleton(itf: &Interface, output_dir: &str) {
    use std::io::Write;

    let mut methods_block = String::new();
    for method in &(itf.methods) {
        make_method_for_skeleton(method, &(itf.name),
                                 &mut methods_block);
    }

    /* read template from file */
    let tpl = include_str!("./grammar/skeleton.tpl");
    let template = Template::new(tpl);

    let mut args = HashMap::<&str, &str>::new();
    let module = &(itf.name).to_lowercase();
    args.insert("module", &module);
    args.insert("interface", &(itf.name));
    args.insert("methods_block", &methods_block);
    let s = template.render(&args);
    /* make skeleton code for rust component */
    let path = format!("{}/{}_skeleton.rs", output_dir, module);
    let mut file = File::create(path).unwrap();

    write!(file, "{}", s).unwrap();
}

fn make_method_for_stub(method: &Method, itf_name: &str,
                        methods: &mut String,
                        funcs: &mut String) {
    use std::fmt::Write;

    /* read template from file */
    let tpl = include_str!("./grammar/method_in_stub.tpl");
    let template = Template::new(tpl);

    let mut decl_args = String::new();
    let mut call_args = String::new();
    for param in &(method.params) {
        let ptype = type_idl_to_rust(&(param.ptype));
        if param.is_sequence {
            write!(decl_args,
                   "{}: *const {}, ", param.name, ptype).unwrap();
            write!(decl_args,
                   "{}_size: core::ffi::c_uint, ", param.name).unwrap();

            write!(call_args, "{}, ", param.name).unwrap();
            write!(call_args, "{}_size, ", param.name).unwrap();
        } else {
            write!(decl_args, "{}: {}, ", param.name, ptype).unwrap();
            write!(call_args, "{}, ", param.name).unwrap();
        }
    }

    let rtype;
    if !method.rtype.is_empty() {
        rtype = format!(" -> {}", type_idl_to_rust(&(method.rtype)));
    } else {
        rtype = String::new();
    }

    let c_method = format!("{interface}_{method}",
                           interface = itf_name.to_lowercase(),
                           method = method.name);

    let mut args = HashMap::<&str, &str>::new();
    args.insert("method", &(method.name));
    args.insert("c_method", &c_method);
    args.insert("rtype", &rtype);
    args.insert("decl_args", &decl_args);
    args.insert("call_args", &call_args);

    write!(methods, "{}", template.render(&args)).unwrap();

    write!(funcs,
    "    pub(crate) fn {c_method}({decl_args}){rtype};",
    c_method = &c_method,
    decl_args = &decl_args,
    rtype = &rtype,
    ).unwrap();
}

fn make_rust_stub(itf: &Interface, output_dir: &str) {
    use std::io::Write;

    let mut methods_block = String::new();
    let mut extern_block = String::new();
    for method in &(itf.methods) {
        make_method_for_stub(method, &(itf.name),
                             &mut methods_block,
                             &mut extern_block);
    }

    /* read template from file */
    let tpl = include_str!("./grammar/stub.tpl");
    let template = Template::new(tpl);

    let mut args = HashMap::<&str, &str>::new();
    let module = &(itf.name).to_lowercase();
    args.insert("module", &module);
    args.insert("interface", &(itf.name));
    args.insert("methods_block", &methods_block);
    args.insert("extern_block", &extern_block);
    let s = template.render(&args);
    /* make skeleton code for rust component */
    let path = format!("{}/{}_stub.rs", output_dir, module);
    let mut file = File::create(path).unwrap();

    write!(file, "{}", s).unwrap();
}

fn be_process(ctx: &Context) {
    for itf in &(ctx.interfaces) {
        //println!("Interface '{}'", itf.name);
        match ctx.option.as_str() {
            "-i" => make_rust_interface(itf, &(ctx.output_dir)),
            "-c" => make_rust_stub(itf, &(ctx.output_dir)),
            "-s" => make_rust_skeleton(itf, &(ctx.output_dir)),
            _ => panic!("bad option"),
        }
    }
}

fn main() {
    let args: Vec<String> = std::env::args().collect();
    if args.len() != 5 {
        panic!("Usage: idl -O output_dir [-i|-c|-s] interface");
    }

    assert!(&args[1] == "-O");
    let output = &args[2];

    let option = &args[3];
    assert!(matches!(option.as_str(), "-i" | "-c" | "-s"));

    let itf = &args[4];

    let filename = format!("./interfaces/{}.idl", itf);
    //println!("interface idl: {}", filename);
    let mut file = File::open(filename).unwrap();
    let mut data = String::new();
    file.read_to_string(&mut data).unwrap();

    let spec: Pairs<Rule> =
        IdlParser::parse(Rule::specification, &data)
            .unwrap_or_else(|e| panic!("{}", e));

    let mut ctx = Context::new();
    ctx.output_dir = output.to_string();
    ctx.option = option.to_string();

    for p in spec {
        fe_process(&p, &mut ctx);
    }

    //println!("\n\n#####################\n\n");
    be_process(&ctx);
}
