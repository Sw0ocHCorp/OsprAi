slint::slint!(import { OsprAiWindow } from "ui/osprai_window.slint";);

fn main() {
    let app = OsprAiWindow::new().unwrap();
    app.run().unwrap();
}