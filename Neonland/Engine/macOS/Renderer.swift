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
        mtkView.clearColor = .init(red: 0, green: 0, blue: 0, alpha: 1)
        mtkView.preferredFramesPerSecond = 60
        
        commandQueue = device.makeCommandQueue()!
        commandQueue.label = "Command Queue"
        
        var globalUniformBuffers = [MTLBuffer]()
        var instanceBuffers = [MTLBuffer]()
        
        for i in 0..<Renderer.maxFramesInFlight {
            let globalUniformsBuffer = device.makeBuffer(length: MemoryLayout<GlobalUniforms>.size, options: .storageModeShared)!
            globalUniformsBuffer.label = "Global Uniforms \(i)"
            globalUniformBuffers.append(globalUniformsBuffer)
            
            let instanceBuffer = device.makeBuffer(length: MemoryLayout<Instance>.size * MAX_INSTANCE_COUNT, options: .storageModeShared)!
            instanceBuffer.label = "Instances \(i)"
            instanceBuffers.append(instanceBuffer)
            
        }
        
        self.globalUniformBuffers = globalUniformBuffers
        self.instanceBuffers = instanceBuffers
        
        let vertexDescriptor = MTLVertexDescriptor()
        
        // Position
        vertexDescriptor.attributes[0].format = .float3
        vertexDescriptor.attributes[0].offset = 0
        vertexDescriptor.attributes[0].bufferIndex = 0
        
        // Normal
        vertexDescriptor.attributes[1].format = .float3
        vertexDescriptor.attributes[1].offset = 12
        vertexDescriptor.attributes[1].bufferIndex = 0
        
        vertexDescriptor.layouts[0].stride = 24
        
        let mdlVertexDescriptor = MTKModelIOVertexDescriptorFromMetal(vertexDescriptor)
        (mdlVertexDescriptor.attributes as! [MDLVertexAttribute])[0].name = MDLVertexAttributePosition;
        (mdlVertexDescriptor.attributes as! [MDLVertexAttribute])[1].name = MDLVertexAttributeNormal;
        
        let allocator = MTKMeshBufferAllocator(device: device)
        
        let playerMDLMesh = MDLMesh(sphereWithExtent: .one / 2,
                                    segments: .init(4, 4),
                                    inwardNormals: false,
                                    geometryType: .triangles,
                                    allocator: allocator)
        
        playerMDLMesh.vertexDescriptor = mdlVertexDescriptor
        
        let enemyMDLMesh = MDLMesh(boxWithExtent: .one,
                                   segments: .one,
                                   inwardNormals: false,
                                   geometryType: .triangles,
                                   allocator: allocator)
        
        enemyMDLMesh.vertexDescriptor = mdlVertexDescriptor
        
        let quadMDLMesh = MDLMesh(planeWithExtent: .one, segments: .one, geometryType: .triangles, allocator: allocator)
        quadMDLMesh.vertexDescriptor = mdlVertexDescriptor
        
        self.meshes = [
            try! MTKMesh(mesh: playerMDLMesh, device: device),
            try! MTKMesh(mesh: enemyMDLMesh, device: device),
            try! MTKMesh(mesh: quadMDLMesh, device: device),
        ]
        
        let library = device.makeDefaultLibrary()!
        library.label = "Default Library"
        
        let renderPipelineDescriptor = MTLRenderPipelineDescriptor()
        renderPipelineDescriptor.label = "Render Pipeline State"
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "vertexFunc")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "fragmentFunc")
        renderPipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        renderPipelineDescriptor.depthAttachmentPixelFormat = mtkView.depthStencilPixelFormat
        
        renderPipelineDescriptor.vertexDescriptor = vertexDescriptor
        
        renderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        
        let depthStencilDescriptor = MTLDepthStencilDescriptor()
        depthStencilDescriptor.isDepthWriteEnabled = true
        depthStencilDescriptor.depthCompareFunction = .less
        depthStencilDescriptor.label = "Depth Stencil State"
        
        depthStencilState = device.makeDepthStencilState(descriptor: depthStencilDescriptor)!
        
        Neon_Start()
    }
    
    func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {
        
    }
    
    func draw(in view: MTKView) {
        frameSemaphore.wait()
        
        pollMouseInput(view: view)
        
        let aspectRatio = Float(view.drawableSize.width / view.drawableSize.height)
        var frameData = Neon_Render(aspectRatio)
        updateUniforms(frameData: &frameData)
        view.clearColor = MTLClearColorMake(Double(frameData.clearColor.x),
                                            Double(frameData.clearColor.y),
                                            Double(frameData.clearColor.z), 1);
        
        let renderPassDescriptor = view.currentRenderPassDescriptor!
        let commandBuffer = commandQueue.makeCommandBuffer()!
        commandBuffer.label = "Command Buffer"
        
        let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        renderCommandEncoder.label = "Render Pipeline Encoder"
        
        renderCommandEncoder.setRenderPipelineState(renderPipelineState)
        renderCommandEncoder.setDepthStencilState(depthStencilState)
        renderCommandEncoder.setFrontFacing(.clockwise)
        renderCommandEncoder.setCullMode(.back)
        
        renderCommandEncoder.setVertexBuffer(instanceBuffers[bufferIndex],
                                             offset: 0,
                                             index: 1)
        
        renderCommandEncoder.setVertexBuffer(globalUniformBuffers[bufferIndex],
                                             offset: 0,
                                             index: 2)
        
        var startOffset = 0
        
        for groupIdx in 0..<frameData.groupCount {
            let meshIndex = Int(groupIdx)
            let mesh = meshes[meshIndex]
            
            let instanceCount = frameData.groupSizes.advanced(by: Int(groupIdx)).pointee
            guard instanceCount > 0 else { continue; }
            
            for (i, vertexBuffer) in mesh.vertexBuffers.enumerated() {
                renderCommandEncoder.setVertexBuffer(vertexBuffer.buffer,
                                                     offset: vertexBuffer.offset,
                                                     index: i)
            }
            
            for submesh in mesh.submeshes {
                renderCommandEncoder.drawIndexedPrimitives(type: submesh.primitiveType,
                                                           indexCount: submesh.indexCount,
                                                           indexType: submesh.indexType,
                                                           indexBuffer: submesh.indexBuffer.buffer,
                                                           indexBufferOffset: submesh.indexBuffer.offset,
                                                           instanceCount: instanceCount,
                                                           baseVertex: 0,
                                                           baseInstance: startOffset)
            }
            
            startOffset += instanceCount
        }
        
        
        renderCommandEncoder.endEncoding()
        commandBuffer.present(view.currentDrawable!)
        
        commandBuffer.addCompletedHandler { [weak self] _ in
            self?.frameSemaphore.signal()
        }
        
        commandBuffer.commit()
    }
    
    func pollMouseInput(view: MTKView) {
        if let window = view.window, window.isKeyWindow {
            var pos = view.convert(window.mouseLocationOutsideOfEventStream, to: nil)
            pos.x = (pos.x / view.bounds.width) * 2 - 1;
            pos.y = (pos.y / view.bounds.height) * 2 - 1;
            
            Neon_UpdateCursorPosition(SIMD2<Float>(Float(pos.x), Float(pos.y)))
        }
    }
    
    func updateUniforms(frameData: inout FrameData) {
        bufferIndex = (bufferIndex + 1) % Renderer.maxFramesInFlight
        
        instanceBuffers[bufferIndex].contents()
            .copyMemory(from: frameData.instances, byteCount: MemoryLayout<Instance>.size * frameData.instanceCount)
        
        globalUniformBuffers[bufferIndex].contents()
            .copyMemory(from: &frameData.globalUniforms, byteCount: MemoryLayout<GlobalUniforms>.size)
        
    }
    
}
