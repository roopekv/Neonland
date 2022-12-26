import MetalKit
import Dispatch
import simd

class Renderer : NSObject, MTKViewDelegate {
    
    static let maxFramesInFlight = 3
    
    let device: MTLDevice
    let commandQueue: MTLCommandQueue
    
    let vertexBuffer: MTLBuffer
    let uniformBuffer: MTLBuffer
    
    let renderPipelineState: MTLRenderPipelineState
    
    let frameSemaphore = DispatchSemaphore(value: maxFramesInFlight)
    
    var frameIndex = 0
    
    let uniformSize = MemoryLayout<Float>.size * 2
    let uniformStride = max(MemoryLayout<Float>.size * 2, 256)
    
    var position: SIMD2<Float> = .zero
    
    init(device: MTLDevice, mtkView: MTKView) {
        self.device = device
        
        let commandQueue = device.makeCommandQueue()!
        
        self.commandQueue = commandQueue
        
        let vertices: [Float] = [
            -1, -1, 1, 0, 0, 1,
             1, -1, 0, 1, 0, 1,
             0,  1, 0, 0, 1, 1
        ]
        
        vertexBuffer = device.makeBuffer(bytes: vertices,
                                         length: MemoryLayout<Float>.stride * vertices.count,
                                         options: .storageModeShared)!
        
        uniformBuffer = device.makeBuffer(length: uniformStride * 3,
                                          options: .storageModeShared)!
        
        let library = device.makeDefaultLibrary()!
        
        let renderPipelineDescriptor = MTLRenderPipelineDescriptor()
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "vertexFunc")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "fragmentFunc")
        renderPipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        
        let vertexDescriptor = MTLVertexDescriptor()
        vertexDescriptor.attributes[0].format = .float2
        vertexDescriptor.attributes[0].offset = 0
        vertexDescriptor.attributes[0].bufferIndex = 0
        
        vertexDescriptor.attributes[1].format = .float4
        vertexDescriptor.attributes[1].offset = MemoryLayout<Float>.stride * 2
        vertexDescriptor.attributes[1].bufferIndex = 0
        
        vertexDescriptor.layouts[0].stride = MemoryLayout<Float>.stride * 6
        
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor
        
        renderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        
    }
    
    func draw(in view: MTKView) {
        frameSemaphore.wait()
        
        Update()
        
        frameIndex = (frameIndex + 1) % Renderer.maxFramesInFlight
        position.x += 0.01
        
        uniformBuffer.contents()
            .advanced(by: frameIndex * uniformStride)
            .copyMemory(from: &position, byteCount: uniformSize)
        
        let renderPassDescriptor = view.currentRenderPassDescriptor!
        
        let commandBuffer = commandQueue.makeCommandBuffer()!
        let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        
        renderCommandEncoder.setRenderPipelineState(renderPipelineState)
        renderCommandEncoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
        renderCommandEncoder.setVertexBuffer(uniformBuffer, offset: frameIndex * uniformStride, index: 1)
        renderCommandEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
        
        renderCommandEncoder.endEncoding()
        commandBuffer.present(view.currentDrawable!)
        
        commandBuffer.addCompletedHandler { [weak self] _ in
            self?.frameSemaphore.signal()
        }
        
        commandBuffer.commit()
    }
    
}
