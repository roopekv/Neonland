import AppKit

class AppDelegate: NSObject, NSApplicationDelegate {
    
    private var window: NSWindow!
    
    func applicationWillFinishLaunching(_ notification: Notification) {
        NSApplication.shared.mainMenu = MenuBar()
    }
    
    func applicationDidFinishLaunching(_ aNotification: Notification) {
        window = .init(contentRect: .zero,
                       styleMask: [.closable, .miniaturizable, .resizable, .titled],
                       backing: .buffered,
                       defer: false)
        
        window.tabbingMode = .disallowed
        window.title = NSRunningApplication.current.localizedName ?? "App"
        window.contentViewController = ViewController()
        
        window.setContentSize(.init(width: 1280, height: 720))
        window.center()
        window.setFrameAutosaveName("MainWindow")
        window.makeKeyAndOrderFront(nil)
        
        
    }
    
    func applicationWillTerminate(_ aNotification: Notification) {
        
    }
    
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }
    
}
