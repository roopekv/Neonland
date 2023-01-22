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
    
    let renderPipelineStates: [MTLRenderPipelineState]
    
    let depthStencilState: MTLDepthStencilState
    let samplerState: MTLSamplerState
    
    let frameSemaphore = DispatchSemaphore(value: maxFramesInFlight)
    
    var bufferIndex = 0
    
    init(mtkView: MTKView) {
        device = MTLCreateSystemDefaultDevice()!
        mtkView.device = device
        mtkView.colorPixelFormat = .bgra8Unorm_srgb
        mtkView.depthStencilPixelFormat = .depth32Float
        mtkView.depthStencilStorageMode = .memoryless
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
        
        var meshes = [MTKMesh?](repeating: nil, count: Int(MeshTypeCount.rawValue))
        
        let meshIdxToName: [UInt32 : String] = [
            PLANE_MESH.rawValue : "plane",
            CROSSHAIR_MESH.rawValue : "crosshair",
            SPREAD_MESH.rawValue : "spread_circle",
            CUBE_MESH.rawValue : "cube",
            SHARD_MESH.rawValue : "shard",
            SPHERE_MESH.rawValue : "sphere"
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
        
        let textureIdxToName: [UInt32 : String] = [
            NO_TEX.rawValue : "blank",
            ENEMIES_REMAINING_TEX.rawValue : "enemies_remaining",
            HP_TEX.rawValue : "hp",
            WAVE_TEX.rawValue : "wave",
            NEONLAND_TEX.rawValue : "neonland",
            NUM_KEYS_TEX.rawValue : "num_keys",
            GROUND1_TEX.rawValue : "ground1",
            GROUND2_TEX.rawValue : "ground2",
            GROUND3_TEX.rawValue : "ground3",
            LEVEL1_BT_TEX.rawValue : "level1_bt",
            LEVEL2_BT_TEX.rawValue : "level2_bt",
            LEVEL3_BT_TEX.rawValue : "level3_bt",
            PAUSED_TEX.rawValue : "paused",
            RESUME_BT_TEX.rawValue : "resume_bt",
            EXIT_BT_TEX.rawValue : "exit_bt",
            QUIT_BT_TEX.rawValue : "quit_bt",
            GAME_OVER_TEX.rawValue : "game_over",
            LEVEL_CLEARED_TEX.rawValue : "level_cleared",
            SPHERE_HEART_TEX.rawValue : "sphere_heart",
            SPHERE_360_SHOTS_TEX.rawValue : "sphere_360_shots"
        ]
        
        for i in 0...9 {
            let tex = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: "\(i)", withExtension: "png")!, options: textureOptions)
            textures[Int(ZERO_TEX.rawValue) + i] = tex
        }
        
        for pair in textureIdxToName {
            let tex = try! textureLoader.newTexture(URL: Bundle.main.url(forResource: pair.value, withExtension: "png")!, options: textureOptions)
            textures[Int(pair.key)] = tex
        }
        
        self.textures = textures.map { $0! }
        
        for (i, tex) in self.textures.enumerated() {
            Neon_UpdateTextureSize(TextureType(UInt32(i)), TexSize(width: Int64(tex.width), height: Int64(tex.height)))
        }

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
        renderPipelineDescriptor.label = "Lit Render Pipeline State"
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "VertexLit")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "FragmentLit")
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
        
        let mainRenderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        
        renderPipelineDescriptor.label = "UI Render Pipeline State"
        renderPipelineDescriptor.vertexFunction = library.makeFunction(name: "VertexUI")
        renderPipelineDescriptor.fragmentFunction = library.makeFunction(name: "FragmentUI")
        let uiRenderPipelineState = try! device.makeRenderPipelineState(descriptor: renderPipelineDescriptor)
        
        var renderPipelineStates = [MTLRenderPipelineState?](repeating: nil, count: Int(ShaderTypeCount.rawValue))
        
        renderPipelineStates[Int(LIT_SHADER.rawValue)] = mainRenderPipelineState;
        renderPipelineStates[Int(UI_SHADER.rawValue)] = uiRenderPipelineState;
        
        self.renderPipelineStates = renderPipelineStates.map { $0! }
        
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
        
        for i in 0..<frameData.audioCount {
            let audioIndex = frameData.audios.advanced(by: i).pointee
            AudioPlayer.shared.play(AudioType(audioIndex))
        }
        
        updateUniforms(frameData: &frameData)
        view.clearColor = MTLClearColorMake(Double(frameData.clearColor.x),
                                            Double(frameData.clearColor.y),
                                            Double(frameData.clearColor.z), 1);
        
        let renderPassDescriptor = view.currentRenderPassDescriptor!
        let commandBuffer = commandQueue.makeCommandBuffer()!
        commandBuffer.label = "Command Buffer"
        
        let renderCommandEncoder = commandBuffer.makeRenderCommandEncoder(descriptor: renderPassDescriptor)!
        renderCommandEncoder.label = "Render Pipeline Encoder"
        
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
        var prevShaderIdx: UInt32 = ShaderTypeCount.rawValue
        var prevMeshIdx: UInt32 = MeshTypeCount.rawValue
        for groupIdx in 0..<Int(frameData.groupCount) {
            let instanceCount = frameData.groupSizes.advanced(by: groupIdx).pointee
            guard instanceCount > 0 else { fatalError("!"); }
            
            let meshIndex = frameData.groupMeshes.advanced(by: groupIdx).pointee
            let mesh = meshes[Int(meshIndex)]
            
            if (meshIndex != prevMeshIdx) {
                for (i, vertexBuffer) in mesh.vertexBuffers.enumerated() {
                    renderCommandEncoder.setVertexBuffer(vertexBuffer.buffer,
                                                         offset: vertexBuffer.offset,
                                                         index: i)
                }
                
                prevMeshIdx = meshIndex
            }
            
            let textureIndex = frameData.groupTextures.advanced(by: groupIdx).pointee
            
            if textureIndex != prevTextureIdx {
                renderCommandEncoder.setFragmentTexture(textures[Int(textureIndex)], index: 0)
                prevTextureIdx = textureIndex
            }
            
            let shaderIdx = frameData.groupShaders.advanced(by: groupIdx).pointee;
            
            if shaderIdx != prevShaderIdx {
                renderCommandEncoder.setRenderPipelineState(renderPipelineStates[Int(shaderIdx)]);
                prevShaderIdx = shaderIdx;
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
