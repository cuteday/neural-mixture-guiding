#pragma once
#include "common.h"
#include "device/cuda.h"

KRR_NAMESPACE_BEGIN

enum class ErrorMetric { MSE, MAPE, SMAPE, RelMSE, Count };

KRR_ENUM_DEFINE(ErrorMetric, {
	{ErrorMetric::MSE, "mse"},
	{ErrorMetric::MAPE, "mape"},
	{ErrorMetric::SMAPE, "smape"},
	{ErrorMetric::RelMSE, "rel_mse"}
});

float calc_metric(const CudaRenderTarget& frame, const Color4f *reference, size_t n_elements, ErrorMetric metric);

KRR_NAMESPACE_END