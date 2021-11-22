// Copyright (c) Recluse Engine 2021.
#ifndef APPLICATION_SETTINGS_GLSL
#define APPLICATION_SETTINGS_GLSL


struct SceneView {
    mat4 view;
    mat4 viewToClip;
};


struct PerMeshTransform {
    mat4 worldToViewClip;
    mat4 world;
    mat4 n;
};


struct PerMaterialSettings {
    uint matId;
    uint enableBump;
    uint lod;
    uint arrayId;
};

#endif // APPLICATION_SETTINGS_GLSL