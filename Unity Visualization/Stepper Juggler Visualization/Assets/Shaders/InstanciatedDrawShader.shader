Shader "Custom/InstanciatedDrawShader"
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
        Tags { "RenderType"="Opaque" }

        CGPROGRAM
        #pragma surface surf Standard vertex:vert addshadow nolightmap
        #pragma instancing_options procedural:setup
        #pragma target 4.5

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

            float4 data = float4(node.position.xyz, 0.5);

            unity_ObjectToWorld._11_21_31_41 = float4(data.w, 0, 0, 0);
            unity_ObjectToWorld._12_22_32_42 = float4(0, data.w, 0, 0);
            unity_ObjectToWorld._13_23_33_43 = float4(0, 0, data.w, 0);
            unity_ObjectToWorld._14_24_34_44 = float4(data.xyz, 1);
            unity_WorldToObject = unity_ObjectToWorld;
            unity_WorldToObject._14_24_34 *= -1;
            unity_WorldToObject._11_22_33 = 1.0f / unity_WorldToObject._11_22_33;

            #endif
        }

        void vert(inout appdata_full v)
        {
            #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            #endif
        }

        void surf(Input IN, inout SurfaceOutputStandard o)
        {
            float2 uv = IN.uv_MainTex;
            o.Albedo = tex2D(_MainTex, uv).rgb * _Color.rgb * IN.Color.rgb;
            //o.Albedo = clamp(float4(0,0,0,0), float4(1,1,1,1), o.Albedo * 1.5);
            //o.Emission = clamp(float4(0,0,0,0), float4(1,1,1,1), o.Albedo * 0.5);

            //o.Emission = float4(1,0,0,1);
            o.Metallic = _Metallic;
            o.Smoothness = _Smoothness;
            o.Normal = UnpackScaleNormal(tex2D(_NormalMap, uv), _NormalScale);
        }

        ENDCG
    }
}