Shader "Shuriken Plus/i Textureless Particle"
{
    Properties
    {
        _MainTex ("Color (RGB) Alpha (A)", 2D) = "white" {}
        _Color("Albedo Color", Color) = (1, 1, 1, 1)
        _Smoothness("Smoothness", Range(0, 1)) = 0
        _Metallic("Metallic", Range(0, 1)) = 0
        _NormalMap("Normal Map", 2D) = "bump" {}
        _NormalScale("Normal Scale", Range(0, 1)) = 1
    }
    SubShader
    {
        Tags { "Queue"="Transparent" "IgnoreProjector"="True" "RenderType"="Transparent" }
        Cull off Blend SrcAlpha One ZWrite Off

        CGPROGRAM
        #pragma surface surf Standard vertex:vert addshadow nolightmap alpha:fade
        #pragma instancing_options procedural:setup
        #pragma target 4.5

        struct appdata_custom
        {
            float4 vertex : POSITION;
            float4 tangent : TANGENT;
            float3 normal : NORMAL;
            float4 texcoord : TEXCOORD0;
            float4 texcoord1 : TEXCOORD1;
            float4 texcoord2 : TEXCOORD2;
            float4 texcoord3 : TEXCOORD3;
            fixed4 color : COLOR;
            uint vertexID : SV_VertexID;
            UNITY_VERTEX_INPUT_INSTANCE_ID
        };

        struct Input
        {
            float2 uv_MainTex;
            half4 Color : COLOR;
        };

        sampler2D _MainTex;
        half4 _Color;
        half _Smoothness;
        half _Metallic;

        sampler2D _NormalMap;
        half _NormalScale;

        struct Node 
        {
            float3 position;
            float3 velocity;
        };

        #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            StructuredBuffer<Node> nodeBuffer;
        #endif

        void setup()
        {
            #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            Node node = nodeBuffer[unity_InstanceID];

            float4 data = float4(node.position.xyz, 0.25);

            unity_ObjectToWorld._11_21_31_41 = float4(data.w, 0, 0, 0);
            unity_ObjectToWorld._12_22_32_42 = float4(0, data.w, 0, 0);
            unity_ObjectToWorld._13_23_33_43 = float4(0, 0, data.w, 0);
            unity_ObjectToWorld._14_24_34_44 = float4(data.xyz, 1);
            unity_WorldToObject = unity_ObjectToWorld;
            unity_WorldToObject._14_24_34 *= -1;
            unity_WorldToObject._11_22_33 = 1.0f / unity_WorldToObject._11_22_33;

            #endif
        }

        void vert(inout appdata_custom v)
        {
            // get the camera basis vectors
            float3 forward = -normalize(UNITY_MATRIX_V._m20_m21_m22);
            float3 up = float3(0, 1, 0); //normalize(UNITY_MATRIX_V._m10_m11_m12);
            float3 right = normalize(UNITY_MATRIX_V._m00_m01_m02);

            // rotate to face camera
            float4x4 rotationMatrix = float4x4(right,   0,
                                             up,      0,
                                             forward, 0,
                                             0, 0, 0, 1);

            //float offset = _Object2World._m22 / 2;
            float offset = 0;
            v.vertex = mul(v.vertex + float4(0, offset, 0, 0), rotationMatrix) + float4(0, -offset, 0, 0);
            v.normal = mul(v.normal, rotationMatrix);
        }

        void surf(Input IN, inout SurfaceOutputStandard o)
        {
            float2 uv = IN.uv_MainTex;

            // textureless particle
            half shape = 0.25;
            half x = length(uv - 0.5) * 2;
            x = 1 - smoothstep(shape, 1, x);
            // textureless particle

            o.Albedo = tex2D(_MainTex, uv).rgb * _Color.rgb * IN.Color.rgb;
            //o.Albedo = clamp(float4(0,0,0,0), float4(1,1,1,1), o.Albedo * 1.5);
            //o.Emission = clamp(float4(0,0,0,0), float4(1,1,1,1), o.Albedo * 0.5);

            //o.Emission = float4(1,0,0,1);

            // COMMENTED OUT 2018_01_29
            o.Metallic = _Metallic;
            o.Smoothness = _Smoothness;
            o.Normal = UnpackScaleNormal(tex2D(_NormalMap, uv), _NormalScale);
            // COMMENTED OUT 2018_01_29

            o.Alpha = x * 0.5;
        }

        ENDCG
    }
}