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

pub(crate) fn provide(ts: TokenStream) -> TokenStream {
    let mut it = ts.into_iter();

    let info = ProvideInfo::parse(&mut it);

    println!("provide {} for {}", info.interface, info.component);

    let method = "ready";
    let sym_name = format!("{interface}_{method}",
                           interface = info.interface,
                           method = method);

    format!(
        "
            #[doc(hidden)]
            #[no_mangle]
            pub extern \"C\" fn {sym_name}() -> bool {{
                {component}::ready()
            }}

            #[no_mangle]
            #[link_section = \"_ksymtab_strings\"]
            static EXPORT_STR_{sym_name}: [u8; {len}+1] = *b\"{sym_name}\\0\";

            #[no_mangle]
            #[link_section = \"_ksymtab\"]
            static EXPORT_SYM_{sym_name}: ExportSymbol = ExportSymbol {{
                value: {sym_name} as *const fn(),
                name: EXPORT_STR_{sym_name}.as_ptr(),
            }};
        ",
        sym_name = sym_name,
        len = sym_name.len(),
        component = info.component,
    )
    .parse()
    .expect("Error parsing formatted string into token stream.")
}
