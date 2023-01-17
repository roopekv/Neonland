import MetalKit
import Metal
import Dispatch
import simd

class Renderer : NSObject, MTKViewDelegate {
    
    static let maxFramesInFlight = 3
    
    let device: MTLDevice
    let commandQueue: MTLCommandQueue
    
    let meshes: [MTKMesh]
    let textures: [MTLTexture]
    
    let globalUniformBuffers: [MTLBuffer]
    let instanceBuffers: [MTLBuffer]
    
    let renderPipelineState: MTLRenderPipelineState
    let depthStencilState: MTLDepthStencilState
    let samplerState: MTLSamplerState
    
    let frameSemaphore = DispatchSemaphore(value: maxFramesInFlight)
    
    var bufferIndex = 0
    
    init(mtkView: MTKView) {
        device = MTLCreateSystemDefaultDevice()!
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm_srgb
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.clearColor = .init(red: 0, green: 0, blue: 0, alpha: 1)
        mtkView.preferredFramesPerSecond = 120
        
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
        
        // Texture Coord
        vertexDescriptor.attributes[2].format = .float2
        vertexDescriptor.attributes[2].offset = 24
        vertexDescriptor.attributes[2].bufferIndex = 0
        
        vertexDescriptor.layouts[0].stride = 32
        
        let mdlVertexDescriptor = MTKModelIOVertexDescriptorFromMetal(vertexDescriptor)
        (mdlVertexDescriptor.attributes as! [MDLVertexAttribute])[0].name = MDLVertexAttributePosition;
        (mdlVertexDescriptor.attributes as! [MDLVertexAttribute])[1].name = MDLVertexAttributeNormal;
        (mdlVertexDescriptor.attributes as! [MDLVertexAttribute])[2].name = MDLVertexAttributeTextureCoordinate;
        
        let allocator = MTKMeshBufferAllocator(device: device)
        
        let sphereMDLMesh = MDLMesh(sphereWithExtent: .one / 2,
                                    segments: .init(4, 4),
                                    inwardNormals: false,
                                    geometryType: .triangles,
                                    allocator: allocator)
        
        sphereMDLMesh.vertexDescriptor = mdlVertexDescriptor
        
        let cubeMDLMesh = MDLMesh(boxWithExtent: .one,
                                  segments: .one,
                                  inwardNormals: false,
                                  geometryType: .triangles,
                                  allocator: allocator)
        
        cubeMDLMesh.vertexDescriptor = mdlVertexDescriptor
        
        var meshes = [MTKMesh?](repeating: nil, count: Int(MeshTypeCount.rawValue))
        
        meshes[Int(SPHERE_MESH.rawValue)] = try! MTKMesh(mesh: sphereMDLMesh, device: device)
        meshes[Int(CUBE_MESH.rawValue)] = try! MTKMesh(mesh: cubeMDLMesh, device: device)
        
        let meshIdxToName: [UInt32 : String] = [
            PLANE_MESH.rawValue : "plane",
            CROSSHAIR_MESH.rawValue : "crosshair",
            SPREAD_MESH.rawValue : "spread_circle"
        ]
        
        for pair in meshIdxToName {
            let asset = MDLAsset(url: Bundle.main.url(forResource: pair.value, withExtension: "obj"),
                                 vertexDescriptor: mdlVertexDescriptor, bufferAllocator: allocator)
            
            let mdlMesh = (asset.childObjects(of: MDLMesh.self) as! [MDLMesh])[0]
            
            meshes[Int(pair.key)] = try! MTKMesh(mesh: mdlMesh, device: device)
        }
        
        self.meshes = meshes.map { $0! }
        
        let textureLoader = MTKTextureLoader(device: device)
        
        let textureOptions: [MTKTextureLoader.Option : Any] = [
            .textureUsage : MTLTextureUsage.shaderRead.rawValue,
            .textureStorageMode : MTLStorageMode.private.rawValue,
            .generateMipmaps : MTKTextureLoader.Option.generateMipmaps,
            .origin : MTKTextureLoader.Origin.topLeft
        ]
        
        var textures = [MTLTexture?](repeating: nil, count: Int(TextureTypeCount.rawValue))
        
        for i in 0...9 {
            textures[Int(ZERO_TEX.rawValue) + i] = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: "\(i)", withExtension: "png")!,
                                                                                 options: textureOptions)
        }
        
        textures[Int(GROUND_TEX.rawValue)] = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: "ground", withExtension: "png")!,
                                                                           options: textureOptions)
        
        textures[Int(NO_TEX.rawValue)] = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: "blank", withExtension: "png")!,
                                                                       options: textureOptions)
        
        textures[Int(SLASH_TEX.rawValue)] = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: "slash", withExtension: "png")!,
                                                                          options: textureOptions)
        
        self.textures = textures.map { $0! }
        
        let samplerDescriptor = MTLSamplerDescriptor()
        samplerDescriptor.normalizedCoordinates = true
        samplerDescriptor.magFilter = .linear
        samplerDescriptor.minFilter = .linear
        samplerDescriptor.mipFilter = .linear
        
        samplerDescriptor.sAddressMode = .repeat
        samplerDescriptor.tAddressMode = .repeat
        samplerDescriptor.label = "Sampler State"
        
        samplerState = device.makeSamplerState(descriptor: samplerDescriptor)!
        
        let library = device.makeDefaultLibrary()!
        library.label = "Default Library"
        
        let renderPipelineDescriptor = MTLRenderPipelineDescriptor()
        renderPipelineDescriptor.label = "Render Pipeline State"
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "vertexFunc")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "fragmentFunc")
        renderPipelineDescriptor.colorAttachments[0].pixelFormat = mtkView.colorPixelFormat
        
        renderPipelineDescriptor.colorAttachments[0].isBlendingEnabled = true
        renderPipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = .sourceAlpha
        renderPipelineDescriptor.colorAttachments[0].sourceAlphaBlendFactor = .sourceAlpha
        
        renderPipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor = .oneMinusSourceAlpha
        renderPipelineDescriptor.colorAttachments[0].destinationAlphaBlendFactor = .oneMinusSourceAlpha
        
        renderPipelineDescriptor.colorAttachments[0].rgbBlendOperation = .add
        renderPipelineDescriptor.colorAttachments[0].alphaBlendOperation = .add
        
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
        
        renderCommandEncoder.setFragmentSamplerState(samplerState, index: 0)
        
        renderCommandEncoder.setVertexBuffer(instanceBuffers[bufferIndex],
                                             offset: 0,
                                             index: 1)
        
        renderCommandEncoder.setVertexBuffer(globalUniformBuffers[bufferIndex],
                                             offset: 0,
                                             index: 2)
        
        var startOffset = 0
        
        var prevTextureIdx: UInt32 = TextureTypeCount.rawValue
        for groupIdx in 0..<Int(frameData.groupCount) {
            let meshIndex = Int(frameData.groupMeshes.advanced(by: groupIdx).pointee)
            let mesh = meshes[meshIndex]
            
            let textureIndex = frameData.groupTextures.advanced(by: groupIdx).pointee
            
            if textureIndex != prevTextureIdx {
                renderCommandEncoder.setFragmentTexture(textures[Int(textureIndex)], index: 0)
                prevTextureIdx = textureIndex
            }
            
            let instanceCount = frameData.groupSizes.advanced(by: groupIdx).pointee
            
            guard instanceCount > 0 else { fatalError("!"); }
            
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
