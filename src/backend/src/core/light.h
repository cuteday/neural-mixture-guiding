#pragma once

#include "common.h"
#include "taggedptr.h"
#include "shape.h"
#include "texture.h"

KRR_NAMESPACE_BEGIN
namespace rt {

struct LightSample {
	Interaction intr;
	Color L;
	float pdf;
};

struct LightSampleContext {
	Vector3f p;
	Vector3f n;
};

class PointLight {
public:
	PointLight() = default;

	PointLight(const Affine3f &transform, const Color &I, float scale = 1) :
		transform(transform), I(I), scale(scale) {}

	KRR_DEVICE LightSample sampleLi(Vector2f u, const LightSampleContext &ctx) const {
		Vector3f p	= transform.translation();
		Vector3f wi = (p - ctx.p).normalized();
		Color Li	= scale * I / (p - ctx.p).squaredNorm();
		return LightSample{Interaction{p}, Li, 1};
	}

	KRR_DEVICE Color L(Vector3f p, Vector3f n, Vector2f uv, Vector3f w) const {
		return Color::Zero();
	}

	KRR_DEVICE float pdfLi(const Interaction &p, const LightSampleContext &ctx) const { return 0; }

private:
	Color I;
	float scale;
	Affine3f transform;
};

class DirectionalLight {
public:
	DirectionalLight() = default;

	DirectionalLight(const Affine3f &transform, const Color &I, float scale = 1) :
		transform(transform), I(I), scale(scale) {}

	KRR_DEVICE LightSample sampleLi(Vector2f u, const LightSampleContext &ctx) const {
		Vector3f wi = transform * Vector3f{0, 0, 1};
		Vector3f p	= ctx.p + wi * 1e7f;
		return LightSample{Interaction{p}, scale * I, 1};
	}

	KRR_DEVICE Color L(Vector3f p, Vector3f n, Vector2f uv, Vector3f w) const {
		return Color::Zero();
	}

	KRR_DEVICE float pdfLi(const Interaction &p, const LightSampleContext &ctx) const { return 0; }

private:
	Color I;
	float scale;
	Affine3f transform;
};

class DiffuseAreaLight {
public:
	DiffuseAreaLight() = default;

	DiffuseAreaLight(Shape &shape, Vector3f Le, bool twoSided = true, float scale = 1.f) :
		shape(shape), Le(Le), twoSided(twoSided), scale(scale) {}

	DiffuseAreaLight(Shape &shape, const rt::TextureData &texture, Vector3f Le = {},
					 bool twoSided = true, float scale = 1.f) :
		shape(shape), texture(texture), Le(Le), twoSided(twoSided), scale(scale) {}

	KRR_DEVICE inline LightSample sampleLi(Vector2f u, const LightSampleContext &ctx) const {
		LightSample ls				= {};
		ShapeSampleContext shapeCtx = {ctx.p, ctx.n};
		ShapeSample ss				= shape.sample(u, shapeCtx);
		DCHECK(!isnan(ss.pdf));
		Interaction &intr = ss.intr;
		intr.wo			  = normalize(ctx.p - intr.p);

		ls.intr = intr;
		ls.pdf	= ss.pdf;
		ls.L	= L(intr.p, intr.n, intr.uv, intr.wo);
		return ls;
	}

	KRR_DEVICE inline Color L(Vector3f p, Vector3f n, Vector2f uv, Vector3f w) const {
		if (!twoSided && dot(n, w) < 0.f) return Color::Zero(); // hit backface

		if (texture.isValid())
			return scale * texture.tex(uv).head<3>();
		else
			return scale * Le;
	}

	KRR_DEVICE float pdfLi(const Interaction &p, const LightSampleContext &ctx) const {
		ShapeSampleContext shapeCtx = {ctx.p, ctx.n};
		return shape.pdf(p, shapeCtx);
	}

private:
	Shape shape;
	rt::TextureData texture{}; // emissive image texture
	Color Le{0};
	bool twoSided{true};
	float scale{1};
};

class InfiniteLight {
public:
	InfiniteLight() = default;

	InfiniteLight(const Affine3f &transform, Color tint, float scale = 1) :
		tint(tint), scale(scale), transform(transform) {}

	InfiniteLight(const Affine3f &transform, const rt::TextureData &image, float scale = 1) :
		image(image), tint(Color::Ones()), scale(scale), transform(transform) {}

	KRR_DEVICE inline LightSample sampleLi(Vector2f u, const LightSampleContext &ctx) const {
		// [TODO] use intensity importance sampling here.
		LightSample ls = {};
		Vector3f wi	   = uniformSampleSphere(u);
		ls.intr		   = Interaction(ctx.p + wi * 1e7f);
		ls.L		   = Li(wi);
		ls.pdf		   = M_INV_4PI;
		return ls;
	}

	KRR_DEVICE float pdfLi(const Interaction &p, const LightSampleContext &ctx) const {
		return M_INV_4PI;
	}

	KRR_DEVICE Color L(Vector3f p, Vector3f n, Vector2f uv, Vector3f w) const {
		return Color::Zero();
	}

	KRR_DEVICE inline Color Li(Vector3f wi) const {
		Color L = tint * scale;
		if (!image.isValid()) return L;
		Vector2f uv = utils::cartesianToSphericalNormalized(transform * wi);
		L *= image.tex(uv).head<3>();
		return L;
	}

private:
	Color tint{1};
	float scale{1};
	Affine3f transform;
	rt::TextureData image{};
};

class Light :
	public TaggedPointer<rt::PointLight, rt::DirectionalLight, rt::DiffuseAreaLight,
						 rt::InfiniteLight> {
public:
	using TaggedPointer::TaggedPointer;

	KRR_DEVICE LightSample sampleLi(Vector2f u, const LightSampleContext &ctx) const {
		auto sampleLi = [&](auto ptr) -> LightSample { return ptr->sampleLi(u, ctx); };
		return dispatch(sampleLi);
	}

	KRR_DEVICE Color L(Vector3f p, Vector3f n, Vector2f uv, Vector3f w) const {
		auto L = [&](auto ptr) -> Vector3f { return ptr->L(p, n, uv, w); };
		return dispatch(L);
	}

	KRR_DEVICE float pdfLi(const Interaction &p, const LightSampleContext &ctx) const {
		auto pdf = [&](auto ptr) -> float { return ptr->pdfLi(p, ctx); };
		return dispatch(pdf);
	}
};

/* You only have OneShot. */
} // namespace rt
KRR_NAMESPACE_END