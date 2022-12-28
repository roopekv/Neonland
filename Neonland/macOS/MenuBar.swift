import AppKit

class MenuBar : NSMenu {
    
    convenience init() {
        self.init(title: "")
    }
    
    override init(title: String) {
        super.init(title: title)
        
        let appItem = NSMenuItem()
        appItem.submenu = makeAppMenu()
        
        let fileItem = NSMenuItem()
        fileItem.submenu = makeFileMenu()
        
        let viewItem = NSMenuItem()
        viewItem.submenu = makeViewMenu()
        
        let windowItem = NSMenuItem()
        windowItem.submenu = makeWindowMenu()
        
        let helpItem = NSMenuItem()
        helpItem.submenu = makeHelpMenu()
        
        self.items = [
            appItem,
            fileItem,
            viewItem,
            windowItem,
            helpItem
        ]
    }
    
    required init(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    private func makeAppMenu() -> NSMenu {
        let name = NSRunningApplication.current.localizedName ?? "App"
        
        // About
        let aboutItem = NSMenuItem(title: "About \(name)",
                                   action: #selector(NSApplication.orderFrontStandardAboutPanel(_:)),
                                   keyEquivalent: "")
        
        // Services
        let servicesItem = NSMenuItem(title: "Services",
                                      action: nil,
                                      keyEquivalent: "")
        servicesItem.submenu = NSMenu()
        NSApplication.shared.servicesMenu = servicesItem.submenu;
        
        // Hide
        let hideItem = NSMenuItem(title: "Hide \(name)",
                                  action: #selector(NSApplication.hide(_:)),
                                  keyEquivalent: "h")
        hideItem.keyEquivalentModifierMask = .command
        
        // Hide Others
        let hideOthersItem = NSMenuItem(title: "Hide Others",
                                        action: #selector(NSApplication.hideOtherApplications(_:)),
                                        keyEquivalent: "h")
        hideOthersItem.keyEquivalentModifierMask = [.command, .shift]
        
        // Show All
        let showAllItem = NSMenuItem(title: "Show All",
                                     action:#selector(NSApplication.unhideAllApplications(_:)),
                                     keyEquivalent: "")
        
        // Quit
        let quitItem = NSMenuItem(title: "Quit \(name)",
                                  action: #selector(NSApplication.terminate(_:)),
                                  keyEquivalent: "q")
        quitItem.keyEquivalentModifierMask = .command
        
        let appMenu = NSMenu();
        appMenu.items = [
            aboutItem,
            .separator(),
            servicesItem,
            .separator(),
            hideItem,
            hideOthersItem,
            showAllItem,
            .separator(),
            quitItem
        ]
        
        return appMenu
    }
    
    private func makeFileMenu() -> NSMenu {
        // Close
        let closeItem = NSMenuItem(title: "Close",
                                   action: #selector(NSWindow.performClose(_:)),
                                   keyEquivalent: "w")
        closeItem.keyEquivalentModifierMask = .command
        
        // File menu
        let fileMenu = NSMenu(title: "File")
        fileMenu.items = [
            closeItem
        ]
        
        return fileMenu
    }
    
    private func makeViewMenu() -> NSMenu {
        // Enter Full Screen
        let fullScreenItem = NSMenuItem(title: "Enter Full Screen",
                                        action: #selector(NSWindow.toggleFullScreen(_:)),
                                        keyEquivalent: "f")
        fullScreenItem.keyEquivalentModifierMask = [.control, .command]
        
        // View menu
        let viewMenu = NSMenu(title: "View")
        viewMenu.items = [
            fullScreenItem
        ]
        
        return viewMenu
    }
    
    private func makeWindowMenu() -> NSMenu {
        // Minimize
        let minimizeItem = NSMenuItem(title: "Minimize",
                                      action: #selector(NSWindow.miniaturize(_:)),
                                      keyEquivalent: "m")
        minimizeItem.keyEquivalentModifierMask = .command
        
        // Zoom
        let zoomItem = NSMenuItem(title: "Zoom",
                                  action: #selector(NSWindow.performZoom(_:)),
                                  keyEquivalent: "")
        
        // Window menu
        let windowMenu = NSMenu(title: "Window")
        NSApplication.shared.windowsMenu = windowMenu;
        windowMenu.items = [
            minimizeItem,
            zoomItem
        ]
        
        return windowMenu
    }
    
    private func makeHelpMenu() -> NSMenu {
        let name = NSRunningApplication.current.localizedName ?? "App"
        
        // App Help
        let helpItem = NSMenuItem(title: "\(name) Help", action: nil, keyEquivalent: "?")
        helpItem.keyEquivalentModifierMask = .command
        
        // Help menu
        let helpMenu = NSMenu(title: "Help")
        NSApplication.shared.helpMenu = helpMenu;
        helpMenu.items = [
            helpItem
        ]
        
        return helpMenu
    }
    
}
