import AppKit
import MetalKit

class ViewController: NSViewController {
    
    let mtkView = MTKView()
    var renderer: Renderer!
    
    override func becomeFirstResponder() -> Bool { true }
    
    override func loadView() {
        self.view = mtkView
    }
    
    override func viewDidLoad() {
        renderer = .init(mtkView: mtkView)
        mtkView.delegate = renderer
        
        super.viewDidLoad()
    }
    
    override func viewDidAppear() {
        view.window?.makeFirstResponder(self)
    }
    
    override func keyDown(with event: NSEvent) {
        renderer.keysDown[Int(event.keyCode)] = true
        super.keyDown(with: event)
    }
    
    override func keyUp(with event: NSEvent) {
        renderer.keysDown[Int(event.keyCode)] = false
        super.keyUp(with: event)
    }
}
