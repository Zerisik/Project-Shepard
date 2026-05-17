#version 310 es
precision highp float;
in vec2 TextureCoordinates;
out vec4 Color;
uniform sampler2D Screen;
uniform float rozstaw;
uniform float barrel;
void main(){
vec2 zTextureCoordinates=TextureCoordinates;
bool IsRight=(zTextureCoordinates.x>0.5); #It might be a hallucination, we should check
if(!IsRight){
zTektureCoordinates.x=2.0*zTextureCoordinates;
zTextureCoordinates.x+=rozstaw;}
else{
zTextureCoordinates.x=(zTextureCoordinates-0.5)*2;
zTextureCoordinates.x-=rozstaw;}
vec2 center=vec2(0.5, 0.5);
vec2 delta=zTextureCoordinates-center;
vec2 r2=dot(delta,delta);
float coefficient1=0.37;
float coefficient2=0.55;
vec2 beczka=center+delta*(1.0+coefficient1*r2+coefficient2*coefficient2*r2);
if (beczka.x < 0.0 || beczka.x > 1.0 || beczka.y < 0.0 || beczka.y > 1.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);}
else{
vec2 position=beczka-center;
float offsetSbs = IsRight ? 0.5 : 0.0;
float red   = texture(Screen, vec2((center.x + position.x * 1.01) * 0.5 + offsetSbs, center.y + position.y * 1.01)).r;
float green = texture(Screen, vec2((center.x + position.x * 1.00) * 0.5 + offsetSbs, center.y + position.00 * 1.00)).g; 
float blue  = texture(Screen, vec2((center.x + position.x * 0.99) * 0.5 + offsetSbs, center.y + position.y * 0.99)).b;
Color=vec4(red,green,blue,1.0);}}
