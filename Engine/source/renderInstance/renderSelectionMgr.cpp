#include "platform/platform.h"
#include "math/util/matrixSet.h"
#include "renderInstance/renderSelectionMgr.h"
#include "scene/sceneManager.h"
#include "scene/sceneRenderState.h"
#include "materials/sceneData.h"
#include "materials/matInstance.h"
#include "materials/matTextureTarget.h"
#include "materials/processedMaterial.h"
#include "postFx/postEffect.h"
#include "gfx/gfxTransformSaver.h"
#include "gfx/gfxDebugEvent.h"


IMPLEMENT_CONOBJECT( RenderSelectionMgr );


ConsoleDocClass( RenderSelectionMgr,
   "@brief A render bin for the selected objects pass.\n\n"
   "When the selection buffer PostEffect is enabled this bin gathers mesh render "
   "instances that are flagged as selected and renders them to the #selection offscreen "
   "render target.\n\n"
   "This render target is then used by the 'SelectionPostFx' PostEffect to "
   "render the highlights and selection effects of the screen.\n\n"
   "@ingroup RenderBin\n" );

const MatInstanceHookType RenderSelectionMgr::SelectionMaterialHook::Type( "Selection" );

RenderSelectionMgr::SelectionMaterialHook::SelectionMaterialHook ( BaseMatInstance *matInst )
   : mSelectionMatInst( NULL )
{
   mSelectionMatInst = (MatInstance*)matInst->getMaterial()->createMatInstance();
   //mSelectionMatInst->getFeaturesDelegate().bind( &SelectionMaterialHook::_overrideFeatures );
   mSelectionMatInst->init(  matInst->getRequestedFeatures(),
                        matInst->getVertexFormat() );
}

RenderSelectionMgr::SelectionMaterialHook::~SelectionMaterialHook()
{
   SAFE_DELETE( mSelectionMatInst );
}

RenderSelectionMgr::RenderSelectionMgr()
   : RenderTexTargetBinManager(  RenderPassManager::RIT_Mesh,
                                 1.0f,
                                 1.0f,
                                 GFXFormatR8G8B8A8,
                                 Point2I( 512, 512 ) )
{
   mNamedTarget.registerWithName( "selection" );
   mTargetSizeType = WindowSize;
}

RenderSelectionMgr::~RenderSelectionMgr()
{
}  

PostEffect* RenderSelectionMgr::getSelectionEffect()
{
   if ( !mSelectionEffect )
      mSelectionEffect = dynamic_cast<PostEffect*>( Sim::findObject( "SelectionPostFx" ) );

   return mSelectionEffect;
}

bool RenderSelectionMgr::isSelectionEnabled()
{
   if ( !mSelectionEffect )
      mSelectionEffect = dynamic_cast<PostEffect*>( Sim::findObject( "SelectionPostFx" ) );

   return mSelectionEffect && mSelectionEffect->isEnabled();
}

void RenderSelectionMgr::addElement( RenderInst *inst )
{
   if ( !isSelectionEnabled() )
      return;

   BaseMatInstance* matInst = getMaterial(inst);
   bool hasSelection = matInst && matInst->hasSelection();
   if ( !hasSelection )
      return;

   internalAddElement(inst);
}
  
void RenderSelectionMgr::render( SceneRenderState *state )
{
   PROFILE_SCOPE( RenderSelectionMgr_Render );

   const U32 binSize = mElementList.size();

   // Don't allow non-diffuse passes.
   if ( !isSelectionEnabled() || !state->isDiffusePass() || binSize == 0) {
	  getSelectionEffect()->setSkip( true );
      return;
   }

   GFXDEBUGEVENT_SCOPE( RenderSelectionMgr_Render, ColorI::GREEN );

   GFXTransformSaver saver;

   // Tell the superclass we're about to render, preserve contents
   const bool isRenderingToTarget = _onPreRender( state, true );

   // Clear all the buffers to black.
   GFX->clear( GFXClearTarget, ColorI::BLACK, 1.0f, 0);

    // Restore transforms
   MatrixSet &matrixSet = getRenderPass()->getMatrixSet();
   matrixSet.restoreSceneViewProjection();

   // init loop data
   SceneData sgData;
   sgData.init( state, SceneData::SelectionBin );

   for( U32 j=0; j<binSize; )
   {
      MeshRenderInst *ri = static_cast<MeshRenderInst*>(mElementList[j].inst);

      setupSGData( ri, sgData );
      sgData.binType = SceneData::SelectionBin;

      BaseMatInstance *mat = ri->matInst;

      U32 matListEnd = j;

      while( mat->setupPass( state, sgData ) )
      {
         U32 a;
         for( a=j; a<binSize; a++ )
         {
            MeshRenderInst *passRI = static_cast<MeshRenderInst*>(mElementList[a].inst);

            if (newPassNeeded(ri, passRI))
               break;

            matrixSet.setWorld(*passRI->objectToWorld);//ADDED
            matrixSet.setView(*passRI->worldToCamera);//ADDED
            matrixSet.setProjection(*passRI->projection);//ADDED
            mat->setTransforms(matrixSet, state);  //ADDED
            mat->setSceneInfo(state, sgData);
            mat->setBuffers(passRI->vertBuff, passRI->primBuff);

            if ( passRI->prim )
               GFX->drawPrimitive( *passRI->prim );
            else
               GFX->drawPrimitive( passRI->primBuffIndex );
         }

         matListEnd = a;
		   setupSGData( ri, sgData );
      }

      // force increment if none happened, otherwise go to end of batch
      j = ( j == matListEnd ) ? j+1 : matListEnd;
   }

   // Finish up.
   if ( isRenderingToTarget )
      _onPostRender();

   getSelectionEffect()->setSkip( false );
}