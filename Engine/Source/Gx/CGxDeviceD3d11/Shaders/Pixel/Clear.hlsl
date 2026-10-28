cbuffer ClearConstants : register(b2) {
    float4 clearColor;
};

float4 main(float4 position : SV_Position) : SV_Target {
    return clearColor;
}
