import AppKit
import MetalKit

class ViewController: NSViewController {
    
    let mtkView = MTKView()
    var renderer: Renderer!
    
    override func loadView() {
        self.view = mtkView
    }
    
    override func viewDidLoad() {
        renderer = .init(mtkView: mtkView)
        mtkView.delegate = renderer
        super.viewDidLoad()
    }
    
}

