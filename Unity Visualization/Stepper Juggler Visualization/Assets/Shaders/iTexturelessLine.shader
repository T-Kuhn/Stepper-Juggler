Shader "Shuriken Plus/i Textureless Line"
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

        struct Parameters 
        {
            int numOfLinesPerParticle;
            int numOfNodes;
            float simulationSpeed;
            float boundingSphereRad;
            float deltaTime;
            float lineMinDist;
        };

        struct Node 
        {
            float3 position;
            float3 velocity;
        };

        struct Line
        {
            int startNodeIndex;
            int endNodeIndex;
            float distance;
        };

        #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            StructuredBuffer<Node> nodeBuffer;
            StructuredBuffer<Line> lineBuffer;
            StructuredBuffer<Parameters> parametersBuffer;
        #endif

        void setup()
        {
            #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED

            /*

            Line l = lineBuffer[unity_InstanceID];
            float4 data = float4(0,0,0,1);

            if(l.startNodeIndex > 0 && l.endNodeIndex > 0)
            {
                Node startNode = nodeBuffer[l.startNodeIndex];
                Node endNode = nodeBuffer[l.endNodeIndex];

                //data = float4(startNode.position.xyz, 0.5);
            }

            unity_ObjectToWorld._11_21_31_41 = float4(data.w, 0, 0, 0);
            unity_ObjectToWorld._12_22_32_42 = float4(0, data.w, 0, 0);
            unity_ObjectToWorld._13_23_33_43 = float4(0, 0, data.w, 0);
            unity_ObjectToWorld._14_24_34_44 = float4(data.xyz, 1);
            unity_WorldToObject = unity_ObjectToWorld;
            unity_WorldToObject._14_24_34 *= -1;
            unity_WorldToObject._11_22_33 = 1.0f / unity_WorldToObject._11_22_33;

            */

            #endif
        }

        void vert(inout appdata_custom v)
        {
            #ifdef UNITY_PROCEDURAL_INSTANCING_ENABLED
            Line l = lineBuffer[unity_InstanceID];

            if(l.startNodeIndex > 0 && l.endNodeIndex > 0)
            {
                Node startNode = nodeBuffer[l.startNodeIndex];
                Node endNode = nodeBuffer[l.endNodeIndex];

                Parameters parameters = parametersBuffer[0];

                switch(v.vertexID){
                    case 0:
                        v.vertex = float4(startNode.position - float3(0, 0.02, 0), 1);
                        break;
                    case 1:
                        v.vertex = float4(endNode.position + float3(0, 0.02, 0), 1);
                        break;
                    case 2:
                        v.vertex = float4(endNode.position - float3(0, 0.02, 0), 1);
                        break;
                    case 3:
                        v.vertex = float4(startNode.position + float3(0, 0.02, 0), 1);
                        break;
                }

                float alpha = (parameters.lineMinDist - l.distance) / parameters.lineMinDist;
                v.color = float4(1,1,1, alpha);
            }
            else
            {
                v.vertex = float4(0, 0, -100, 1);
                //v.color = float4(1,1,1,0);
            }

            #endif
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

            o.Metallic = _Metallic;
            o.Smoothness = _Smoothness;
            o.Normal = UnpackScaleNormal(tex2D(_NormalMap, uv), _NormalScale);

            //o.Alpha = x;
            o.Alpha = 0.2 * IN.Color.a;
        }

        ENDCG
    }
}