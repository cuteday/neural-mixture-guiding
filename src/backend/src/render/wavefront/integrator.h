#pragma once
#include "window.h"
#include "scene.h"
#include "camera.h"
#include "file.h"
#include "renderpass.h"

#include "device/buffer.h"
#include "device/context.h"
#include "device/optix.h"
#include "device/cuda.h"
#include "workqueue.h"

KRR_NAMESPACE_BEGIN

class WavefrontPathTracer : public RenderPass {
public:
	using SharedPtr = std::shared_ptr<WavefrontPathTracer>;
	KRR_REGISTER_PASS_DEC(WavefrontPathTracer);

	WavefrontPathTracer() = default;
	~WavefrontPathTracer() = default;

	void resize(const Vector2i& size) override;
	void setScene(Scene::SharedPtr scene) override;
	void beginFrame(RenderContext* context) override;
	void render(RenderContext *context) override;
	void renderUI() override;

	void initialize();

	string getName() const override { return "WavefrontPathTracer"; }

	void handleHit();
	void handleMiss();
	void generateScatterRays();
	void generateCameraRays(int sampleId);
	void traceClosest(int depth);
	void traceShadow();

	KRR_CALLABLE RayQueue* currentRayQueue(int depth) { return rayQueue[depth & 1]; }
	KRR_CALLABLE RayQueue* nextRayQueue(int depth) { return rayQueue[(depth & 1) ^ 1]; }

	template <typename... Args>
	KRR_DEVICE_FUNCTION void debugPrint(uint pixelId, const char *fmt, Args &&...args);
	
	OptixBackend *backend{ };
	Camera::CameraData* camera{ };
	LightSampler lightSampler;

	// work queues
	RayQueue* rayQueue[2]{ };	// switching bewteen current and next queue
	MissRayQueue* missRayQueue{ };
	HitLightRayQueue* hitLightRayQueue{ };
	ShadowRayQueue* shadowRayQueue{ };
	ScatterRayQueue* scatterRayQueue{ };
	PixelStateBuffer* pixelState;

	// path tracing parameters
	int maxQueueSize;
	int samplesPerPixel{ 1 };
	int maxDepth{ 10 };
	float probRR{ 0.8 };
	bool enableNEE{ };
	bool enableMIS{true};
	bool debugOutput{ };
	bool enableClamp{false};
	uint debugPixel{ };
	float clampMax{ 1e3f };

	friend void to_json(json &j, const WavefrontPathTracer &p) { 
		j = json{ 
			{ "nee", p.enableNEE }, 
			{ "mis", p.enableMIS },
			{ "max_depth", p.maxDepth },
			{ "rr", p.probRR },
			{ "enable_clamp", p.enableClamp },
			{ "clamp_max", p.clampMax }
		};
	}

	friend void from_json(const json &j, WavefrontPathTracer &p) {
		p.enableNEE	  = j.value("nee", true);
		p.enableMIS	  = j.value("mis", true);
		p.maxDepth	  = j.value("max_depth", 10);
		p.probRR	  = j.value("rr", 0.8);
		p.enableClamp = j.value("enable_clamp", false);
		p.clampMax	  = j.value("clamp_max", 1e3f);
	}
};

KRR_NAMESPACE_END