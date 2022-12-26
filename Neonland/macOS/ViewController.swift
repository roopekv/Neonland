import AppKit
import MetalKit

class ViewController: NSViewController {
    
    private var mtkView: MTKView!
    private var renderer: Renderer!
    
    override func loadView() {
        mtkView = MTKView()
        self.view = mtkView
    }
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        let device = MTLCreateSystemDefaultDevice()!
        
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm_srgb
        
        renderer = .init(device: device, mtkView: mtkView)
        mtkView.delegate = renderer
    }
    
}

