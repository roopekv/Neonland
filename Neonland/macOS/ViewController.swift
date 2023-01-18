import AppKit
import MetalKit
import Carbon.HIToolbox.Events

class ViewController: NSViewController {
    
    let mtkView = SceneView()
    var renderer: Renderer!
    
    var keysDown = [Bool](repeating: false, count: Int(UInt16.max))
    
    var cursorHidden = false
    
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
    
    override func mouseEntered(with event: NSEvent) {
        if (!cursorHidden) {
            NSCursor.hide()
            cursorHidden = true
        }
        super.mouseEntered(with: event)
    }
    
    override func mouseExited(with event: NSEvent) {
        if (cursorHidden) {
            NSCursor.unhide()
            cursorHidden = false
        }
        super.mouseExited(with: event)
    }
    
    override func mouseDown(with event: NSEvent) {
        Neon_UpdateMouseDown(true);
        super.mouseDown(with: event)
    }
    
    override func mouseUp(with event: NSEvent) {
        Neon_UpdateMouseDown(false);
        super.mouseUp(with: event)
    }
    
    override func keyDown(with event: NSEvent) {
        let keyCode = UInt16(event.keyCode)
        keysDown[Int(event.keyCode)] = true
        Neon_UpdateDirectionalInput(MoveDir)
        
        if let num = getNumber(keyCode: keyCode){
            Neon_UpdateNumberKeyPressed(num)
        }
    }
    
    override func keyUp(with event: NSEvent) {
        keysDown[Int(event.keyCode)] = false
        Neon_UpdateDirectionalInput(MoveDir)
    }
    
    func getNumber(keyCode: UInt16) -> Int32? {
        switch keyCode {
        case UInt16(kVK_ANSI_1):
            return 1
        case UInt16(kVK_ANSI_2):
            return 2
        case UInt16(kVK_ANSI_3):
            return 3
        default:
            return nil
        }
    }
}
