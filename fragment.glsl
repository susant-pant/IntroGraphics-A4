// ==========================================================================
// Vertex program for barebones GLFW boilerplate
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================
#version 410

// interpolated colour received from vertex stage
in vec2 uv;
in vec2 pixelPos;

// first output is mapped to the framebuffer's colour index by default
out vec4 FragmentColour;

uniform float xPos;
uniform float yPos;
uniform float zPos;

struct Plane{
	vec3 normal;
	vec3 point;
	vec3 color;
	float p;
	float specCol;
	float ref;
};

struct Sphere{
	vec3 center;
	float radius;
	vec3 color;
	float p;
	float specCol;
	float ref;
};

struct Triangle{
	vec3 p0;
	vec3 p1;
	vec3 p2;
	vec3 color;
	float p;
	float specCol;
	float ref;
};

struct Light{
	vec3 pos;
	vec3 color;
};

uniform Triangle triangles[50];
uniform Sphere spheres[10];
uniform Plane planes[2];
uniform Light lights[1];

float PI = 3.1415926535897932384626433832795;
float FOV = PI/3.0;
float minDist = 100000.0;
vec3 x_axis = vec3(1.0,0.0,0.0);
vec3 y_axis = vec3(0.0,1.0,0.0);
vec3 z_axis = vec3(0.0,0.0,1.0);

float intersectPlane(vec3 dir, Plane plane, vec3 start){
	vec3 disp = plane.point - start;
	float numer = dot(plane.normal, disp);
	float denom = dot(dir, plane.normal);
	if (denom == 0.0){	
		return -1.0;
	}
	float t = numer/denom;
	return t;
}

float intersectSphere(vec3 dir, Sphere sphere, vec3 start){
	float a = dot(dir, dir);
	float b = 2.0 * (dot(start, dir) - dot(sphere.center, dir));
	float c = (-2.0*dot(start, sphere.center)) + dot(start, start) + dot(sphere.center, sphere.center) - (sphere.radius * sphere.radius);
	float discriminant = b*b - 4.0*a*c;
	if (discriminant < 0){
		return -1.0;
	}
	float t1 = (-b + sqrt(discriminant)) / 2.0*a;
	float t2 = (-b - sqrt(discriminant)) / 2.0*a;
	float retVal = min(t1,t2);
	if (retVal < 0.0){
		retVal = max(t1,t2);
	}
	return retVal;
}

float intersectTriangle(vec3 dir, Triangle triangle, vec3 start){
	vec3 s = start - triangle.p0;
	vec3 e1 = triangle.p1 - triangle.p0;
	vec3 e2 = triangle.p2 - triangle.p0;

	float tNumer = determinant(mat3(s,e1,e2));
	float uNumer = determinant(mat3(-dir,s,e2));
	float vNumer = determinant(mat3(-dir,e1,s));
	float denom = 1.0/determinant(mat3(-dir,e1,e2));

	float t = tNumer * denom;
	float u = uNumer * denom;
	float v = vNumer * denom;

	if ((u+v) < 1.0 && (u+v) > 0.0 && u > 0.0 && u < 1.0 && v > 0.0 && v < 1.0){
		return t;
	}
	else {
		return -1.0;
	}
}

vec3 normalTriangle(Triangle triangle){
	vec3 vector1 = triangle.p1 - triangle.p0;
	vec3 vector2 = triangle.p2 - triangle.p0;
	vec3 normal = cross(vector1, vector2);
	return normal;
}

uniform bool lightType = true;
uniform bool scene3 = false;

vec3 getColor(vec3 sectPoint, int objType, int currObj, vec3 dir){
	//objType: 0 is Triangle, 1 is Sphere, 2 is Plane
	float t;
	bool shadowed = false;
	vec3 retCol = vec3(1.0);
	for (int i = 0; i < lights.length(); i++){
		vec3 ray = lights[i].pos - sectPoint;
		vec3 lightRay = normalize(ray);
		float rayLength = sqrt(dot(ray, ray));
		float dist = rayLength;
		float border = 0.001;
		if(scene3){
			border = 0.0;
		}

		for (int j = 0; j < triangles.length(); j++){
			if (!(objType == 0 && currObj == j)){
				t = intersectTriangle(lightRay, triangles[j], sectPoint);
			}
			if (t > border && t <= rayLength && t < dist){
				dist = t;
				shadowed = true;
			}
		}
		for (int j = 0; j < spheres.length(); j++){
			if (!(objType == 1 && currObj == j)){
				t = intersectSphere(lightRay, spheres[j], sectPoint);
			}
			if (t > border && t <= rayLength && t < dist){
				dist = t;
				shadowed = true;
			}
		}
		for (int j = 0; j < planes.length(); j++){
			if (!(objType == 2 && currObj == j)){
				t = intersectPlane(lightRay, planes[j], sectPoint);
			}
			if (t > border && t <= rayLength && t < dist){
				dist = t;
				shadowed = true;
			}
		}

		if (dist < 100000.0){
			vec3 objCol;
			vec3 normal;
			float specVal;
			float pVal = 0.0;
			float specCol = 0.0;
			switch(objType) {
				case 0 : // triangles
					normal = normalize(normalTriangle(triangles[currObj]));
					objCol = triangles[currObj].color;
					pVal = triangles[currObj].p;
					specVal = triangles[currObj].specCol;
					break;
				case 1 : // spheres
					normal = normalize(sectPoint - spheres[currObj].center);
					objCol = spheres[currObj].color;
					pVal = spheres[currObj].p;
					specVal = spheres[currObj].specCol;
					break;
				case 2 : // planes
					normal = normalize(planes[currObj].normal);
					objCol = planes[currObj].color;
					pVal = planes[currObj].p;
					specVal = planes[currObj].specCol;
					break;
			}
			vec3 intensity = lights[i].color;
			vec3 intensityDiff = intensity;
			vec3 intensitySpec = intensity;
			if (shadowed){
				intensityDiff *= (atan(dist * 0.5)/(PI * 0.5));
				intensitySpec = 0.4 * intensityDiff;
			}
			vec3 ambient = intensity*0.2;
			vec3 specular = vec3(1.0);//(specVal)*vec3(1.0) + (1.0 - specVal)*objCol;
			vec3 h = normalize(-dir + lightRay);
			vec3 diffuse = objCol * (ambient + (intensityDiff * max(0.0, dot(normal, lightRay))));
			retCol *= diffuse + (intensitySpec * specular * pow(max(0.0, dot(h,normal)), pVal));
		}
		else {
			return vec3(1.0);
		}
	}
	return retCol;
}

vec3 getReflection(vec3 dir, vec3 startColor, float refIndex, vec3 normal, vec3 sectPoint){
	vec3 retCol = startColor * (1.0 - refIndex);
	float reflCoeff = refIndex;

	vec3 reflRay;

	vec3 objCol;
	float objRef;
	vec3 objNorm;
	int objType;
	int iVal;

	float border = 0.001;
	if(scene3){
		border = 0.0;
	}

	int iter = 0;
	while ((iter < 20) && (refIndex > 0.0)){
		reflRay = normalize(dir - (2.0 * normal * dot(dir, normal)));
		float dist = 100000.0;

		float t;
		for (int i = 0; i < triangles.length(); i++){
			t = intersectTriangle(reflRay, triangles[i], sectPoint);
			if (t > border && t < dist){
				dist = t;
				objRef = triangles[i].ref;
				objNorm = normalize(normalTriangle(triangles[i]));
				objType = 0;
				iVal = i;
			}
		}
		for (int i = 0; i < spheres.length(); i++){
			t = intersectSphere(reflRay, spheres[i], sectPoint);
			if (t > border && t < dist){
				dist = t;
				objRef = spheres[i].ref;
				objNorm = normalize(sectPoint - spheres[i].center);
				objType = 1;
				iVal = i;
			}
		}
		for (int i = 0; i < planes.length(); i++){
			t = intersectPlane(reflRay, planes[i], sectPoint);
			if (t > border && t < dist){
				dist = t;
				objRef = planes[i].ref;
				objNorm = normalize(planes[i].normal);
				objType = 2;
				iVal = i;
			}
		}
		if (dist < 100000.0){
			dir = reflRay;
			sectPoint = sectPoint + (dist * dir);
			normal = objNorm;

			objCol = getColor(sectPoint, objType, iVal, dir);
			
			refIndex = objRef;
			retCol += (objCol * (1.0 - refIndex) * reflCoeff);
			reflCoeff *= refIndex;

			iter++;
		}
		else {
			objCol = vec3(0.0);
			refIndex = objRef;
			retCol += (objCol * (1.0 - refIndex) * reflCoeff);
			return retCol;
		}
	}
	return retCol;
}

vec3 getClosestIntersection(vec3 dir, vec3 origin){
	vec3 color;
	float t;
	int objType;
	int iVal;
	float reflVal;
	vec3 normal;
	for (int i = 0; i < triangles.length(); i++){
		t = intersectTriangle(dir, triangles[i], origin);
		if (t > 0.0 && t < minDist){
			minDist = t;
			objType = 0;
			iVal = i;
			reflVal = triangles[i].ref;
			normal = normalize(normalTriangle(triangles[i]));
		}
	}
	for (int i = 0; i < spheres.length(); i++){
		t = intersectSphere(dir, spheres[i], origin);
		if (t > 0.0 && t < minDist){
			minDist = t;
			objType = 1;
			iVal = i;
			reflVal = spheres[i].ref;
			normal = normalize((origin + (minDist*dir)) - spheres[i].center);
		}
	}
	for (int i = 0; i < planes.length(); i++){
		t = intersectPlane(dir, planes[i], origin);
		if (t > 0.0 && t < minDist){
			minDist = t;
			objType = 2;
			iVal = i;
			reflVal = planes[i].ref;
			normal = normalize(planes[i].normal);
		}
	}
	if(minDist < 100000.0){
		vec3 intersectPoint = origin + (minDist*dir);
		color = getColor(intersectPoint, objType, iVal, dir);
		color = getReflection(dir, color, reflVal, normal, intersectPoint);
		return color;
	}
	return vec3(0.0);
}

mat3 rotationMatrixY(float theta){
	return mat3(cos(theta), 0.0, 	sin(theta),
				0.0,		1.0, 	0.0,
				-sin(theta), 0.0,	cos(theta));
}

mat3 rotationMatrixX(float theta){
	return mat3(1.0,	0.0, 		0.0,
				0.0,	cos(theta), -sin(theta),
				0.0,	sin(theta),	cos(theta));
}

uniform float xRot;
uniform float yRot;
uniform bool brokenFocus;

void main(void){
	vec3 color = vec3(0.0);

	vec3 origin = vec3(0.0, 0.0, 0.0);
	origin += vec3(xPos, yPos, zPos);
	//origin *= rotationMatrixY(yRot); // camera doesn't do the thing

	float focal = -1.0 / tan(FOV * 0.5);
	vec3 direction = normalize(vec3(pixelPos, focal));
	direction *= rotationMatrixX(xRot) * rotationMatrixY(yRot);

	vec3 colorOrig = getClosestIntersection(direction, origin);
	//uncomment the chunk below for DoF attempt
	/*
	if (brokenFocus && !scene3){
		vec3 origin1 = vec3(origin.x - 0.3, origin.y, origin.z);
		vec3 direction1 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot)) * rotationMatrixY(yRot - PI/90.0);
		vec3 color1 = getClosestIntersection(direction1, origin1);

		vec3 origin2 = vec3(origin.x + 0.3, origin.y, origin.z);
		vec3 direction2 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot)) * rotationMatrixY(yRot + PI/90.0);
		vec3 color2 = getClosestIntersection(direction2, origin2);

		vec3 origin3 = vec3(origin.x, origin.y + 0.3, origin.z);
		vec3 direction3 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot - PI/90.0)) * rotationMatrixY(yRot);
		vec3 color3 = getClosestIntersection(direction3, origin3);

		vec3 origin4 = vec3(origin.x, origin.y - 0.3, origin.z);
		vec3 direction4 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot + PI/90.0)) * rotationMatrixY(yRot);
		vec3 color4 = getClosestIntersection(direction4, origin4);

		vec3 origin5 = vec3(origin.x - 0.212132034, origin.y - 0.212132034, origin.z);
		vec3 direction5 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot + PI/127.279220827)) * rotationMatrixY(yRot - PI/127.279220827);
		vec3 color5 = getClosestIntersection(direction5, origin5);

		vec3 origin6 = vec3(origin.x + 0.212132034, origin.y - 0.212132034, origin.z);
		vec3 direction6 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot + PI/127.279220827)) * rotationMatrixY(yRot + PI/127.279220827);
		vec3 color6 = getClosestIntersection(direction6, origin6);

		vec3 origin7 = vec3(origin.x - 0.212132034, origin.y + 0.212132034, origin.z);
		vec3 direction7 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot - PI/127.279220827)) * rotationMatrixY(yRot - PI/127.279220827);
		vec3 color7 = getClosestIntersection(direction7, origin7);

		vec3 origin8 = vec3(origin.x + 0.212132034, origin.y + 0.212132034, origin.z);
		vec3 direction8 = normalize(vec3(pixelPos, focal) * rotationMatrixX(xRot - PI/127.279220827)) * rotationMatrixY(yRot + PI/127.279220827);
		vec3 color8 = getClosestIntersection(direction8, origin8);

		vec3 colorx = (((color1 + color2)/2.0 + (color3 + color4)/2.0)/2.0 + ((color5 + color8)/2.0 + (color6 + color7)/2.0)/2.0)/2.0;
		vec3 colory = (((color1 + color3)/2.0 + (color2 + color4)/2.0)/2.0 + ((color5 + color6)/2.0 + (color8 + color7)/2.0)/2.0)/2.0;
		vec3 colorz = (((color1 + color4)/2.0 + (color2 + color3)/2.0)/2.0 + ((color5 + color7)/2.0 + (color6 + color8)/2.0)/2.0)/2.0;
		color = ((colorx + colory)/2.0 + (colory + colorz)/2.0)/2.0;
		color += (colorx + colorz)/2.0;
		color = (colorOrig + 10*(color))/21.0;
	}
	else {
		color = colorOrig;
	}
	*/
	color = colorOrig; // comment this out as well
	FragmentColour = vec4(color, 1.0);
}
