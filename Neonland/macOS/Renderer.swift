import MetalKit
import Metal
import Dispatch
import simd

class Renderer : NSObject, MTKViewDelegate {
    
    static let maxFramesInFlight = 3
    
    let device: MTLDevice
    let commandQueue: MTLCommandQueue
    
    let meshes: [MTKMesh]
    let globalUniformBuffers: [MTLBuffer]
    let instanceBuffers: [MTLBuffer]
    
    let renderPipelineState: MTLRenderPipelineState
    let depthStencilState: MTLDepthStencilState
    
    let frameSemaphore = DispatchSemaphore(value: maxFramesInFlight)
    
    var bufferIndex = 0
    
    init(mtkView: MTKView) {
        device = MTLCreateSystemDefaultDevice()!
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm_srgb
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.clearColor = .init(red: 0, green: 0, blue: 0.2, alpha: 1)
        mtkView.preferredFramesPerSecond = 120
        
        commandQueue = device.makeCommandQueue()!
        commandQueue.label = "Command Queue"
        
        let allocator = MTKMeshBufferAllocator(device: device)
        
        let playerMDLMesh = MDLMesh(sphereWithExtent: .init(1, 1, 1),
                                    segments: .init(4, 4),
                                    inwardNormals: false,
                                    geometryType: .triangles,
                                    allocator: allocator)
        
        let enemyMDLMesh = MDLMesh(boxWithExtent: .one,
                                   segments: .one,
                                   inwardNormals: false,
                                   geometryType: .triangles,
                                   allocator: allocator)
        
        var meshes = [MTKMesh]()
        meshes.append(try! MTKMesh(mesh: playerMDLMesh, device: device))
        meshes.append(try! MTKMesh(mesh: enemyMDLMesh, device: device))
        self.meshes = meshes
        
        var globalUniformBuffers = [MTLBuffer]()
        var instanceBuffers = [MTLBuffer]()
        
        for i in 0..<Renderer.maxFramesInFlight {
            let globalUniformsBuffer = device.makeBuffer(length: MemoryLayout<GlobalUniforms>.size, options: .storageModeShared)!
            globalUniformsBuffer.label = "Global Uniforms \(i)"
            globalUniformBuffers.append(globalUniformsBuffer)
            
            let instanceBuffer = device.makeBuffer(length: MemoryLayout<Instance>.size * MAX_ENTITY_COUNT, options: .storageModeShared)!
            instanceBuffer.label = "Instance Uniforms \(i)"
            instanceBuffers.append(instanceBuffer)
            
        }
        
        self.globalUniformBuffers = globalUniformBuffers
        self.instanceBuffers = instanceBuffers
        
        let library = device.makeDefaultLibrary()!
        library.label = "Default Library"
        
        let renderPipelineDescriptor = MTLRenderPipelineDescriptor()
        renderPipelineDescriptor.label = "Render Pipeline State"
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "vertexFunc")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "fragmentFunc")
        renderPipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        renderPipelineDescriptor.depthAttachmentPixelFormat = mtkView.depthStencilPixelFormat
        
        let vertexDescriptor = MTKMetalVertexDescriptorFromModelIO(meshes[0].vertexDescriptor)!
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor
        
        renderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        
        let depthStencilDescriptor = MTLDepthStencilDescriptor()
        depthStencilDescriptor.isDepthWriteEnabled = true
        depthStencilDescriptor.depthCompareFunction = .less
        depthStencilDescriptor.label = "Depth Stencil State"
        
        depthStencilState = device.makeDepthStencilState(descriptor: depthStencilDescriptor)!
        
        OnStart()
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        
    }
    
    func draw(in view: MTKView) {
        frameSemaphore.wait()
        
        updateUniforms(view: view)
        
        let renderPassDescriptor = view.currentRenderPassDescriptor!
        let commandBuffer = commandQueue.makeCommandBuffer()!
        commandBuffer.label = "Command Buffer"
        
        let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        renderCommandEncoder.label = "Render Pipeline Encoder"
        
        renderCommandEncoder.setRenderPipelineState(renderPipelineState)
        renderCommandEncoder.setDepthStencilState(depthStencilState)
        renderCommandEncoder.setFrontFacing(.counterClockwise)
        renderCommandEncoder.setCullMode(.back)
        
        renderCommandEncoder.setVertexBuffer(globalUniformBuffers[bufferIndex],
                                             offset: 0,
                                             index: 2)
        
        renderCommandEncoder.setVertexBuffer(instanceBuffers[bufferIndex],
                                             offset: 0,
                                             index: 1)
        
        for (i, vertexBuffer) in meshes[0].vertexBuffers.enumerated() {
            renderCommandEncoder.setVertexBuffer(vertexBuffer.buffer,
                                                 offset: vertexBuffer.offset,
                                                 index: i)
        }
        
        for submesh in meshes[0].submeshes {
            let indexBuffer = submesh.indexBuffer
            renderCommandEncoder.drawIndexedPrimitives(type: submesh.primitiveType,
                                                       indexCount: submesh.indexCount,
                                                       indexType: submesh.indexType,
                                                       indexBuffer: indexBuffer.buffer,
                                                       indexBufferOffset: indexBuffer.offset)
        }
        
        renderCommandEncoder.endEncoding()
        commandBuffer.present(view.currentDrawable!)
        
        commandBuffer.addCompletedHandler { [weak self] _ in
            self?.frameSemaphore.signal()
        }
        
        commandBuffer.commit()
    }
    
    func updateUniforms(view: MTKView) {
        let aspectRatio = Float(view.drawableSize.width / view.drawableSize.height)
        var frameData = OnRender(aspectRatio)
        
        bufferIndex = (bufferIndex + 1) % Renderer.maxFramesInFlight
        
        globalUniformBuffers[bufferIndex].contents()
            .copyMemory(from: &frameData.globalUniforms, byteCount: MemoryLayout<GlobalUniforms>.size)
        
        instanceBuffers[bufferIndex].contents()
            .copyMemory(from: frameData.instances, byteCount: MemoryLayout<Instance>.size * frameData.instanceCount)
    }
    
}
