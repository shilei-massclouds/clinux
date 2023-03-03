//! Stub for [{{interface}}]

use kernel::interfaces::{{module}}::{{interface}};

struct Stub;

impl ILib for Stub {
{{methods_block}}
}

pub(crate) fn get_{{interface}}() -> impl {{interface}} {
    Stub {}
}

extern "C" {
{{extern_block}}
}
