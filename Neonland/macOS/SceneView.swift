import MetalKit

class SceneView: MTKView {
    private var trackingArea: NSTrackingArea?
    
    override func updateTrackingAreas() {
        if let trackingArea {
            self.removeTrackingArea(trackingArea)
        }
        
        trackingArea = NSTrackingArea(rect: self.bounds,
                                      options: [.mouseEnteredAndExited, .mouseMoved, .activeInActiveApp],
                                      owner: self,
                                      userInfo: nil)
        
        if let trackingArea {
            self.addTrackingArea(trackingArea)
        }
    }
}
