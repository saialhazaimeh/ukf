#include <benchmark/benchmark.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include "Types.h"
#include "StateVector.h"
#include "MeasurementVector.h"

enum MyStateVectorFields {
    AngularVelocity,
    Altitude,
    Velocity,
    Attitude
};

using MyStateVector = UKF::StateVector<
    UKF::Field<Velocity, UKF::Vector<3>>,
    UKF::Field<AngularVelocity, UKF::Vector<3>>,
    UKF::Field<Attitude, UKF::Quaternion>,
    UKF::Field<Altitude, real_t>
>;

enum MyFields {
    StaticPressure,
    DynamicPressure,
    Accelerometer,
    Gyroscope
};

using MV_Fixed = UKF::FixedMeasurementVector<
    UKF::Field<Accelerometer, UKF::Vector<3>>,
    UKF::Field<Gyroscope, UKF::Vector<3>>,
    UKF::Field<StaticPressure, real_t>,
    UKF::Field<DynamicPressure, real_t>
>;

template <> template <>
UKF::Vector<3> MV_Fixed::expected_measurement
<MyStateVector, Accelerometer>(const MyStateVector &state) {
    return state.get_field<Attitude>() * UKF::Vector<3>(0, 0, -9.8);
}

template <> template <>
UKF::Vector<3> MV_Fixed::expected_measurement
<MyStateVector, Gyroscope>(const MyStateVector &state) {
    return state.get_field<AngularVelocity>();
}

template <> template <>
real_t MV_Fixed::expected_measurement
<MyStateVector, StaticPressure>(const MyStateVector &state) {
    return 101.3 - 1.2*(state.get_field<Altitude>() / 100.0);
}

template <> template <>
real_t MV_Fixed::expected_measurement
<MyStateVector, DynamicPressure>(const MyStateVector &state) {
    return 0.5 * 1.225 * state.get_field<Velocity>().squaredNorm();
}

using MV_Dynamic = UKF::DynamicMeasurementVector<
    UKF::Field<Accelerometer, UKF::Vector<3>>,
    UKF::Field<Gyroscope, UKF::Vector<3>>,
    UKF::Field<StaticPressure, real_t>,
    UKF::Field<DynamicPressure, real_t>
>;

template <> template <>
UKF::Vector<3> MV_Dynamic::expected_measurement
<MyStateVector, Accelerometer>(const MyStateVector &state) {
    return state.get_field<Attitude>() * UKF::Vector<3>(0, 0, -9.8);
}

template <> template <>
UKF::Vector<3> MV_Dynamic::expected_measurement
<MyStateVector, Gyroscope>(const MyStateVector &state) {
    return state.get_field<AngularVelocity>();
}

template <> template <>
real_t MV_Dynamic::expected_measurement
<MyStateVector, StaticPressure>(const MyStateVector &state) {
    return 101.3 - 1.2*(state.get_field<Altitude>() / 100.0);
}

template <> template <>
real_t MV_Dynamic::expected_measurement
<MyStateVector, DynamicPressure>(const MyStateVector &state) {
    return 0.5 * 1.225 * state.get_field<Velocity>().squaredNorm();
}

/*
Tests to compare set/get performance between fixed and dynamic measurement vectors.
*/
void MeasurementVectorFixed_SetGetField(benchmark::State& state) {
    MV_Fixed test_measurement;
    while(state.KeepRunning()) {
        test_measurement.set_field<Accelerometer>(UKF::Vector<3>(1, 2, 3));
        benchmark::DoNotOptimize(test_measurement.get_field<Accelerometer>());
    }
}

BENCHMARK(MeasurementVectorFixed_SetGetField);

void MeasurementVectorDynamic_SetGetField(benchmark::State& state) {
    MV_Dynamic test_measurement;
    while(state.KeepRunning()) {
        test_measurement.set_field<Accelerometer>(UKF::Vector<3>(1, 2, 3));
        benchmark::DoNotOptimize(test_measurement.get_field<Accelerometer>());
    }
}

BENCHMARK(MeasurementVectorDynamic_SetGetField);

void MeasurementVectorFixed_SigmaPointGeneration(benchmark::State& state) {
    MyStateVector test_state;
    MV_Fixed test_measurement;

    test_measurement.set_field<Accelerometer>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<Gyroscope>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<StaticPressure>(0);
    test_measurement.set_field<DynamicPressure>(0);

    test_state.set_field<Velocity>(UKF::Vector<3>(1, 2, 3));
    test_state.set_field<AngularVelocity>(UKF::Vector<3>(1, 0, 0));
    test_state.set_field<Attitude>(UKF::Quaternion(1, 0, 0, 0));
    test_state.set_field<Altitude>(1000);

    MyStateVector::CovarianceMatrix covariance = MyStateVector::CovarianceMatrix::Zero();
    covariance.diagonal() << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0;

    MyStateVector::SigmaPointDistribution sigma_points = test_state.calculate_sigma_point_distribution(covariance);

    while(state.KeepRunning()) {
        benchmark::DoNotOptimize(test_measurement.calculate_sigma_point_distribution<MyStateVector>(sigma_points));
    }
}

BENCHMARK(MeasurementVectorFixed_SigmaPointGeneration);

void MeasurementVectorDynamic_SigmaPointGeneration(benchmark::State& state) {
    MyStateVector test_state;
    MV_Dynamic test_measurement;

    test_measurement.set_field<Accelerometer>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<Gyroscope>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<StaticPressure>(0);
    test_measurement.set_field<DynamicPressure>(0);

    test_state.set_field<Velocity>(UKF::Vector<3>(1, 2, 3));
    test_state.set_field<AngularVelocity>(UKF::Vector<3>(1, 0, 0));
    test_state.set_field<Attitude>(UKF::Quaternion(1, 0, 0, 0));
    test_state.set_field<Altitude>(1000);

    MyStateVector::CovarianceMatrix covariance = MyStateVector::CovarianceMatrix::Zero();
    covariance.diagonal() << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0;

    MyStateVector::SigmaPointDistribution sigma_points = test_state.calculate_sigma_point_distribution(covariance);

    while(state.KeepRunning()) {
        benchmark::DoNotOptimize(test_measurement.calculate_sigma_point_distribution<MyStateVector>(sigma_points));
    }
}

BENCHMARK(MeasurementVectorDynamic_SigmaPointGeneration);

void MeasurementVectorFixed_FullMeasurementCalculation(benchmark::State& state) {
    MyStateVector test_state;
    MV_Fixed test_measurement;

    test_measurement.set_field<Accelerometer>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<Gyroscope>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<StaticPressure>(0);
    test_measurement.set_field<DynamicPressure>(0);

    test_state.set_field<Velocity>(UKF::Vector<3>(1, 2, 3));
    test_state.set_field<AngularVelocity>(UKF::Vector<3>(1, 0, 0));
    test_state.set_field<Attitude>(UKF::Quaternion(1, 0, 0, 0));
    test_state.set_field<Altitude>(1000);

    MyStateVector::CovarianceMatrix covariance = MyStateVector::CovarianceMatrix::Zero();
    covariance.diagonal() << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0;

    MyStateVector::SigmaPointDistribution sigma_points = test_state.calculate_sigma_point_distribution(covariance);

    MV_Fixed::SigmaPointDistribution<MyStateVector> measurement_sigma_points;
    MV_Fixed mean_measurement;
    MV_Fixed::SigmaPointDeltas<MyStateVector> sigma_point_deltas;
    MV_Fixed::CovarianceMatrix calculated_covariance;

    while(state.KeepRunning()) {
        measurement_sigma_points = test_measurement.calculate_sigma_point_distribution<MyStateVector>(sigma_points);
        mean_measurement = test_measurement.calculate_sigma_point_mean<MyStateVector>(measurement_sigma_points);
        sigma_point_deltas = mean_measurement.calculate_sigma_point_deltas<MyStateVector>(measurement_sigma_points);
        calculated_covariance = mean_measurement.calculate_sigma_point_covariance<MyStateVector>(sigma_point_deltas);
    }
}

BENCHMARK(MeasurementVectorFixed_FullMeasurementCalculation);

void MeasurementVectorDynamic_FullMeasurementCalculation(benchmark::State& state) {
    MyStateVector test_state;
    MV_Dynamic test_measurement;

    test_measurement.set_field<Accelerometer>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<Gyroscope>(UKF::Vector<3>(0, 0, 0));
    test_measurement.set_field<StaticPressure>(0);
    test_measurement.set_field<DynamicPressure>(0);

    test_state.set_field<Velocity>(UKF::Vector<3>(1, 2, 3));
    test_state.set_field<AngularVelocity>(UKF::Vector<3>(1, 0, 0));
    test_state.set_field<Attitude>(UKF::Quaternion(1, 0, 0, 0));
    test_state.set_field<Altitude>(1000);

    MyStateVector::CovarianceMatrix covariance = MyStateVector::CovarianceMatrix::Zero();
    covariance.diagonal() << 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0;

    MyStateVector::SigmaPointDistribution sigma_points = test_state.calculate_sigma_point_distribution(covariance);

    MV_Dynamic::SigmaPointDistribution<MyStateVector> measurement_sigma_points(
        test_measurement.size(), MyStateVector::num_sigma());
    MV_Dynamic mean_measurement(test_measurement.size());
    MV_Dynamic::SigmaPointDeltas<MyStateVector> sigma_point_deltas(test_measurement.size(), MyStateVector::num_sigma());
    MV_Dynamic::CovarianceMatrix calculated_covariance(test_measurement.size(), test_measurement.size());

    while(state.KeepRunning()) {
        measurement_sigma_points = test_measurement.calculate_sigma_point_distribution<MyStateVector>(sigma_points);
        mean_measurement = test_measurement.calculate_sigma_point_mean<MyStateVector>(measurement_sigma_points);
        sigma_point_deltas = mean_measurement.calculate_sigma_point_deltas<MyStateVector>(measurement_sigma_points);
        calculated_covariance = mean_measurement.calculate_sigma_point_covariance<MyStateVector>(sigma_point_deltas);
    }
}

BENCHMARK(MeasurementVectorDynamic_FullMeasurementCalculation);

template <> MV_Fixed::CovarianceVector MV_Fixed::measurement_covariance = MV_Fixed::CovarianceVector();

void MeasurementVectorFixed_MeasurementCovariance(benchmark::State& state) {
    MV_Fixed test_measurement;

    MV_Fixed::measurement_covariance.set_field<Accelerometer>(UKF::Vector<3>(1, 2, 3));
    MV_Fixed::measurement_covariance.set_field<Gyroscope>(UKF::Vector<3>(4, 5, 6));
    MV_Fixed::measurement_covariance.set_field<StaticPressure>(7);
    MV_Fixed::measurement_covariance.set_field<DynamicPressure>(8);

    while(state.KeepRunning()) {
        benchmark::DoNotOptimize(test_measurement.calculate_measurement_covariance());
    }
}

BENCHMARK(MeasurementVectorFixed_MeasurementCovariance);

template <> MV_Dynamic::CovarianceVector MV_Dynamic::measurement_covariance = MV_Dynamic::CovarianceVector();

void MeasurementVectorDynamic_MeasurementCovariance(benchmark::State& state) {
    MV_Dynamic test_measurement;

    MV_Dynamic::measurement_covariance.set_field<Accelerometer>(UKF::Vector<3>(1, 2, 3));
    MV_Dynamic::measurement_covariance.set_field<Gyroscope>(UKF::Vector<3>(4, 5, 6));
    MV_Dynamic::measurement_covariance.set_field<StaticPressure>(7);
    MV_Dynamic::measurement_covariance.set_field<DynamicPressure>(8);

    while(state.KeepRunning()) {
        benchmark::DoNotOptimize(test_measurement.calculate_measurement_covariance());
    }
}

BENCHMARK(MeasurementVectorDynamic_MeasurementCovariance);
