// SPDX-License-Identifier: GPL-2.0

use proc_macro::{token_stream, Delimiter, Group, Literal, TokenStream, TokenTree};
use std::fmt::Write;

use crate::helpers::*;

pub(crate) fn provide(ts: TokenStream) -> TokenStream {
    format!(
        "
            #[no_mangle]
            #[link_section = \"_ksymtab_strings\"]
            static EXPORT_STR: [u8; 6] = *b\"ready\\0\";

            #[no_mangle]
            #[link_section = \"_ksymtab\"]
            static EXPORT_SYM: ExportSymbol = ExportSymbol {{
                value: RustHello::ready as *const fn(),
                name: EXPORT_STR.as_ptr(),
            }};
        "
    )
    .parse()
    .expect("Error parsing formatted string into token stream.")
}
