// SPDX-License-Identifier: GPL-2.0

use proc_macro::{token_stream, Delimiter, Group, Literal, TokenStream, TokenTree};
use std::fmt::Write;

use crate::helpers::*;

#[derive(Debug, Default)]
struct ProvideInfo {
    interface: String,
    component: String,
}

impl ProvideInfo {
    fn parse(it: &mut token_stream::IntoIter) -> Self {
        let mut info = ProvideInfo::default();

        const EXPECTED_KEYS: &[&str] = &[
            "interface",
            "component",
        ];
        const REQUIRED_KEYS: &[&str] = &["interface", "component"];
        let mut seen_keys = Vec::new();

        loop {
            let key = match it.next() {
                Some(TokenTree::Ident(ident)) => ident.to_string(),
                Some(_) => panic!("Expected Ident or end"),
                None => break,
            };

            if seen_keys.contains(&key) {
                panic!(
                    "Duplicated key \"{}\". Keys can only be specified once.",
                    key
                );
            }

            assert_eq!(expect_punct(it), ':');

            match key.as_str() {
                "interface" => info.interface = expect_ident(it),
                "component" => info.component = expect_ident(it),
                _ => panic!(
                    "Unknown key \"{}\". Valid keys are: {:?}.",
                    key, EXPECTED_KEYS
                ),
            }

            assert_eq!(expect_punct(it), ',');

            seen_keys.push(key);
        }

        expect_end(it);

        for key in REQUIRED_KEYS {
            if !seen_keys.iter().any(|e| e == key) {
                panic!("Missing required key \"{}\".", key);
            }
        }

        let mut ordered_keys: Vec<&str> = Vec::new();
        for key in EXPECTED_KEYS {
            if seen_keys.iter().any(|e| e == key) {
                ordered_keys.push(key);
            }
        }

        if seen_keys != ordered_keys {
            panic!(
                "Keys are not ordered as expected. Order them like: {:?}.",
                ordered_keys
            );
        }

        info
    }
}

fn export_symbol_for_method(
    interface: &String, method: &String,
    decl_args: &String, rtype: &Option<String>,
    call_args: &String, buffer: &mut String) {

    let symbol = format!("{interface}_{method}",
                         interface = interface,
                         method = method);

    let mut signature = format!("fn {symbol}({decl_args})",
                                symbol = symbol,
                                decl_args = decl_args);
    if let Some(t) = &rtype {
        write!(&mut signature, " -> {t}", t = t);
    }

    let call_method = format!("{method}({call_args})",
                              method = method,
                              call_args = call_args);

    write!(
        buffer,
        "
            #[doc(hidden)]
            #[no_mangle]
            pub extern \"C\" {signature} {{
                if let Some(m) = unsafe {{ &__MOD }} {{
                    return m.{call_method};
                }}
                // Todo: Implement panic
                loop {{}}
            }}

            #[no_mangle]
            #[link_section = \"_ksymtab_strings\"]
            static EXPORT_STR_{symbol}: [u8; {len}+1] = *b\"{symbol}\\0\";

            #[no_mangle]
            #[link_section = \"_ksymtab\"]
            static EXPORT_SYM_{symbol}: ExportSymbol = ExportSymbol {{
                value: {symbol} as *const fn(),
                name: EXPORT_STR_{symbol}.as_ptr(),
            }};
        ",
        signature = signature,
        call_method = call_method,
        symbol = symbol,
        len = symbol.len(),
    )
    .unwrap();
}

pub(crate) fn provide(ts: TokenStream) -> TokenStream {
    let mut it = ts.into_iter();

    let info = ProvideInfo::parse(&mut it);

    println!("provide {} for {}", info.interface, info.component);

    let mut buffer = String::new();

    {
        let method = String::from("ready");
        let decl_args = String::from("");
        let call_args = String::from("");
        let rtype = Some(String::from("bool"));

        export_symbol_for_method(&info.interface,
                                 &method, &decl_args, &rtype,
                                 &call_args, &mut buffer);
    }

    {
        let method = String::from("name");
        let decl_args = String::from("");
        let call_args = String::from("");
        let rtype = Some(String::from("*const core::ffi::c_char"));

        export_symbol_for_method(&info.interface,
                                 &method, &decl_args, &rtype,
                                 &call_args, &mut buffer);
    }

    buffer.parse().expect("Error parsing formatted string into token stream.")
}
