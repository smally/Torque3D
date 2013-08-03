#ifndef _TEXTARGETBIN_MGR_H_
#include "renderInstance/renderTexTargetBinManager.h"
#endif

class PostEffect;

class RenderSelectionMgr : public RenderTexTargetBinManager
{
   typedef RenderTexTargetBinManager Parent;

public:

   RenderSelectionMgr();
   virtual ~RenderSelectionMgr();

   /// Returns true if the selection post effect is
   /// enabled and the selection buffer should be updated.
   bool isSelectionEnabled();

   /// Returns the glow post effect.
   PostEffect* getSelectionEffect();

   // RenderBinManager
   virtual void addElement( RenderInst *inst );
   virtual void render( SceneRenderState *state );

   // ConsoleObject
   DECLARE_CONOBJECT( RenderSelectionMgr );

protected:

   class SelectionMaterialHook : public MatInstanceHook
   {
   public:

      SelectionMaterialHook( BaseMatInstance *matInst );
      virtual ~SelectionMaterialHook();

      virtual BaseMatInstance *getMatInstance() { return mSelectionMatInst; }

      virtual const MatInstanceHookType& getType() const { return Type; }

      /// Our material hook type.
      static const MatInstanceHookType Type;

   protected:

      static void _overrideFeatures(   ProcessedMaterial *mat,
                                       U32 stageNum,
                                       MaterialFeatureData &fd,
                                       const FeatureSet &features );

      BaseMatInstance *mSelectionMatInst;
   };

   SimObjectPtr<PostEffect> mSelectionEffect;
};