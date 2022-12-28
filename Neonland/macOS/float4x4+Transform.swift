import simd

extension float4x4 {
    
    init(scale: SIMD3<Float>) {
        self.init(diagonal: .init(scale.x, scale.y, scale.z, 1))
    }
    
    init(translation t: SIMD3<Float>) {
        self.init(.init(1, 0, 0, 0),
                  .init(0, 1, 0, 0),
                  .init(0, 0, 1, 0),
                  .init(t.x, t.y, t.z, 1))
    }
    
    init(verticalFoVInDegrees: Float, aspectRatio: Float, near: Float, far: Float) {
        let fov = verticalFoVInDegrees * Float.pi / 180
        
        let scale = 1 / tan(fov / 2)
        let x = scale / aspectRatio
        let y = scale
        
        let zRange = far - near
        
        let z = -(far + near) / zRange
        let w = -2 * far * near / zRange
        
        self.init(.init(x, 0, 0, 0),
                  .init(0, y, 0, 0),
                  .init(0, 0, z, -1),
                  .init(0, 0, w, 0))
    }
}
