// kalman_bindings.cpp
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>           // ← magic: enables Eigen ↔ numpy conversion
#include <pybind11/stl.h>             // for std::vector etc if needed

#include "KalmanFilter.hpp"

namespace py = pybind11;
using namespace pybind11::literals;  // for _a syntax

PYBIND11_MODULE(kalman, m) {
    m.doc() = "Kalman filter bindings for stock/time-series prediction";

    py::class_<Estimator, std::unique_ptr<Estimator>>(m, "Estimator")
        .def("predict", &Estimator::predict)
        .def("update", &Estimator::update, "z"_a)
        .def_property_readonly("state", &Estimator::state)
        .def_property_readonly("covariance", &Estimator::covariance);

    py::class_<KalmanFilter, Estimator>(m, "KalmanFilter")
        .def(py::init<const Eigen::MatrixXd&, const Eigen::MatrixXd&,
                      const Eigen::MatrixXd&, const Eigen::MatrixXd&,
                      const Eigen::MatrixXd&>(),
             "F"_a, "Q"_a, "H"_a, "R"_a, "B"_a)

        .def("init", &KalmanFilter::init, "x0"_a, "P0"_a)

        .def("predict", &KalmanFilter::predict)
        .def("update", &KalmanFilter::update, "z"_a)

        .def_property_readonly("state", &KalmanFilter::state)
        .def_property_readonly("covariance", &KalmanFilter::covariance)

        // Convenience method for 1D price smoothing
        .def("update_price", [](KalmanFilter& self, double price) {
            Eigen::VectorXd z(1);
            z << price;
            self.update(z);
            self.predict();
            return self.state()(0);  // return predicted/updated position
        })

        .def("get_prediction_and_variance", [](const KalmanFilter& self) -> py::tuple {
            auto s = self.state();
            auto P = self.covariance();
            return py::make_tuple(s(0), P(0,0));
        });
}
