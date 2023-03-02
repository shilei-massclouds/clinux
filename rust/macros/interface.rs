// SPDX-License-Identifier: GPL-2.0

use proc_macro::{token_stream, TokenStream, TokenTree};

use crate::helpers::*;

#[derive(Debug, Default)]
struct SymbolInfo {
    symbol: String,
}

impl SymbolInfo {
    fn parse(it: &mut token_stream::IntoIter) -> Self {
        let mut info = SymbolInfo::default();

        const EXPECTED_KEYS: &[&str] = &[
            "symbol",
        ];
        const REQUIRED_KEYS: &[&str] = &["symbol"];
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
                "symbol" => info.symbol = expect_ident(it),
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

pub(crate) fn export_symbol(ts: TokenStream) -> TokenStream {
    let mut it = ts.into_iter();

    let info = SymbolInfo::parse(&mut it);

    println!("export {}", info.symbol);

    format!(
        "
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
        symbol = info.symbol,
        len = info.symbol.len(),
    )
    .parse()
    .expect("Error parsing formatted string into token stream.")
}
