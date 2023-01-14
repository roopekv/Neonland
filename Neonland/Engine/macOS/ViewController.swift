import AppKit
import MetalKit
import Carbon.HIToolbox.Events

class ViewController: NSViewController {
    
    let mtkView = MTKView()
    var renderer: Renderer!
    
    var keysDown = [Bool](repeating: false, count: Int(UInt16.max))
    
    var MoveDir: SIMD2<Float> {
        var dir = SIMD2<Float>.zero
        
        if keysDown[kVK_ANSI_W] || keysDown[kVK_UpArrow] {
            dir.y += 1
        }
        
        if keysDown[kVK_ANSI_S] || keysDown[kVK_DownArrow] {
            dir.y -= 1
        }
        
        if keysDown[kVK_ANSI_D] || keysDown[kVK_RightArrow] {
            dir.x += 1
        }
        
        if keysDown[kVK_ANSI_A] || keysDown[kVK_LeftArrow] {
            dir.x -= 1
        }
        
        return dir;
    }
    
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
    
    override func mouseDown(with event: NSEvent) {
        Neon_UpdateMouseDown(true);
        super.mouseDown(with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
        Neon_UpdateMouseDown(false);
        super.mouseDown(with: event)
    }
    
    override func keyDown(with event: NSEvent) {
        keysDown[Int(event.keyCode)] = true
        Neon_UpdateDirectionalInput(MoveDir)
    }
    
    override func keyUp(with event: NSEvent) {
        keysDown[Int(event.keyCode)] = false
        Neon_UpdateDirectionalInput(MoveDir)
    }
}
