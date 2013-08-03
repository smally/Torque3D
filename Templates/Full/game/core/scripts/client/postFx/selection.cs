singleton GFXStateBlockData( PFX_DefaultSelectionStateBlock : PFX_DefaultStateBlock)  
{  
   zDefined = true;  
   zEnable = false;  
   zWriteEnable = false;  
        
   samplersDefined = true;  
   samplerStates[0] = SamplerClampLinear;  
   //samplerStates[1] = SamplerClampLinear;
   //samplerStates[1] = SamplerWrapPoint;  
};  
  
singleton ShaderData( PFX_SelectionShader )  
{     
   DXVertexShaderFile   = "shaders/common/postFx/postFxV.hlsl";  
   DXPixelShaderFile    = "shaders/common/postFx/selectionShaderP.hlsl";  
           
   //OGLVertexShaderFile  = "shaders/common/postFx/gl//postFxV.glsl";  
   //OGLPixelShaderFile   = "shaders/common/postFx/gl/passthruP.glsl";  
        
   samplerNames[0] = "$inputTex";  
     
   pixVersion = 3.0;  
};  

singleton PostEffect( SelectionPostFX )  
{  
   renderTime = "PFXAfterDiffuse";  

   shader = PFX_SelectionShader;  
   stateBlock = PFX_DefaultSelectionStateBlock;  
   texture[0] = "#selection";   
   texture[1] = "$backBuffer";  
   target = "$backBuffer";  
   
   isEnabled = true;  
};

/// Just here for debug visualization of the 
/// SSAO mask texture used during lighting.
singleton PostEffect( SelectionVizPostFx )
{      
   allowReflectPass = false;
        
   shader = PFX_PassthruShader;
   stateBlock = PFX_DefaultStateBlock;
   
   texture[0] = "#selection";
   
   target = "$backbuffer";
};
