fn main() {
    let config = slint_build::CompilerConfiguration::new()
        .with_style("fluent-dark".to_owned());
    slint_build::compile_with_config("ui/osprai_window.slint", config).unwrap();
}