import MetalKit
import Dispatch
import simd

class Renderer : NSObject, MTKViewDelegate {
    
    static let maxFramesInFlight = 3
    
    let mtkView: MTKView
    
    let device: MTLDevice
    let commandQueue: MTLCommandQueue
    
    let mesh: MTKMesh
    let uniformBuffer: MTLBuffer
    
    let renderPipelineState: MTLRenderPipelineState
    let depthStencilState: MTLDepthStencilState
    
    let frameSemaphore = DispatchSemaphore(value: maxFramesInFlight)
    
    var frameIndex = 0
    
    static let uniformSize = MemoryLayout<VertexUniforms>.size
    static let uniformStride = max(uniformSize, 256)
    
    var uniforms = VertexUniforms(offset: .init(0, 0, 0), projection: .init(1))
    
    init(mtkView: MTKView) {
        self.mtkView = mtkView
        device = MTLCreateSystemDefaultDevice()!
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm_srgb
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.clearColor = .init(red: 0, green: 0, blue: 0.2, alpha: 1)
    
        commandQueue = device.makeCommandQueue()!
        
        let allocator = MTKMeshBufferAllocator(device: device)
        let mdlMesh = MDLMesh(sphereWithExtent: .init(1, 1, 1),
                              segments: .init(24, 24),
                              inwardNormals: false,
                              geometryType: .triangles,
                              allocator: allocator)
        
//        let mdlMesh = MDLMesh(boxWithExtent: .one,
//                              segments: .one,
//                              inwardNormals: false,
//                              geometryType: .triangles,
//                              allocator: allocator)
        
        mesh = try! MTKMesh(mesh: mdlMesh, device: device)
        
        uniformBuffer = device.makeBuffer(length: Renderer.uniformStride * 3,
                                          options: .storageModeShared)!
        
        let library = device.makeDefaultLibrary()!
        
        let renderPipelineDescriptor = MTLRenderPipelineDescriptor()
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "vertexFunc")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "fragmentFunc")
        renderPipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        renderPipelineDescriptor.depthAttachmentPixelFormat = mtkView.depthStencilPixelFormat
        
        let vertexDescriptor = MTKMetalVertexDescriptorFromModelIO(mesh.vertexDescriptor)
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor
        
        renderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        
        let depthStencilDescriptor = MTLDepthStencilDescriptor()
        depthStencilDescriptor.isDepthWriteEnabled = true
        depthStencilDescriptor.depthCompareFunction = .less
        
        depthStencilState = device.makeDepthStencilState(descriptor: depthStencilDescriptor)!
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        
    }
    
    func draw(in view: MTKView) {
        frameSemaphore.wait()
        
        Update()
        
        frameIndex = (frameIndex + 1) % Renderer.maxFramesInFlight
        uniforms.offset.x += 0.001
        
        let aspectRatio = Float(view.drawableSize.width / view.drawableSize.height)
        uniforms.projection = float4x4(verticalFoVInDegrees: 60,
                                       aspectRatio: aspectRatio,
                                       near: 0.1,
                                       far: 100)
        
        uniforms.projection *= float4x4(translation: -.init(0, 0, 10))
        
        uniformBuffer.contents()
            .advanced(by: frameIndex * Renderer.uniformStride)
            .copyMemory(from: &uniforms, byteCount: Renderer.uniformSize)
        
        let renderPassDescriptor = view.currentRenderPassDescriptor!
        
        let commandBuffer = commandQueue.makeCommandBuffer()!
        let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        
        renderCommandEncoder.setRenderPipelineState(renderPipelineState)
        renderCommandEncoder.setDepthStencilState(depthStencilState)
        renderCommandEncoder.setFrontFacing(.counterClockwise)
        renderCommandEncoder.setCullMode(.back)
        
        for (i, vertexBuffer) in mesh.vertexBuffers.enumerated() {
            renderCommandEncoder.setVertexBuffer(vertexBuffer.buffer,
                                                 offset: vertexBuffer.offset,
                                                 index: i)
        }
        
        renderCommandEncoder.setVertexBuffer(uniformBuffer,
                                             offset: frameIndex * Renderer.uniformStride,
                                             index: 1)
        
        for submesh in mesh.submeshes {
            let indexBuffer = submesh.indexBuffer
            renderCommandEncoder.drawIndexedPrimitives(type: submesh.primitiveType,
                                                       indexCount: submesh.indexCount,
                                                       indexType: submesh.indexType,
                                                       indexBuffer: indexBuffer.buffer,
                                                       indexBufferOffset: indexBuffer.offset)
        }
        renderCommandEncoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 3)
        
        renderCommandEncoder.endEncoding()
        commandBuffer.present(view.currentDrawable!)
        
        commandBuffer.addCompletedHandler { [weak self] _ in
            self?.frameSemaphore.signal()
        }
        
        commandBuffer.commit()
    }
    
}
