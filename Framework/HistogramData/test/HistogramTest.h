#ifndef MANTID_HISTOGRAMDATA_HISTOGRAMTEST_H_
#define MANTID_HISTOGRAMDATA_HISTOGRAMTEST_H_

#include <cxxtest/TestSuite.h>

#include "MantidHistogramData/Histogram.h"
#include "MantidHistogramData/LinearGenerator.h"

using Mantid::HistogramData::Histogram;
using Mantid::HistogramData::HistogramX;
using Mantid::HistogramData::HistogramY;
using Mantid::HistogramData::HistogramE;
using Mantid::HistogramData::getHistogramXMode;
using Mantid::HistogramData::Points;
using Mantid::HistogramData::BinEdges;
using Mantid::HistogramData::Counts;
using Mantid::HistogramData::CountVariances;
using Mantid::HistogramData::CountStandardDeviations;
using Mantid::HistogramData::Frequencies;
using Mantid::HistogramData::FrequencyVariances;
using Mantid::HistogramData::FrequencyStandardDeviations;
using Mantid::HistogramData::LinearGenerator;

class HistogramTest : public CxxTest::TestSuite {
public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running other tests
  static HistogramTest *createSuite() { return new HistogramTest(); }
  static void destroySuite(HistogramTest *suite) { delete suite; }

  void test_construction_Points_Counts() {
    TS_ASSERT_THROWS_NOTHING(
        Histogram hist(Histogram::XMode::Points, Histogram::YMode::Counts));
  }

  void test_construction_BinEdges_Counts() {
    TS_ASSERT_THROWS_NOTHING(
        Histogram hist(Histogram::XMode::BinEdges, Histogram::YMode::Counts));
  }

  void test_construction_Points_Frequencies() {
    TS_ASSERT_THROWS_NOTHING(Histogram hist(Histogram::XMode::Points,
                                            Histogram::YMode::Frequencies));
  }

  void test_construction_BinEdges_Frequencies() {
    TS_ASSERT_THROWS_NOTHING(Histogram hist(Histogram::XMode::BinEdges,
                                            Histogram::YMode::Frequencies));
  }

  void test_construct_from_Points() {
    TS_ASSERT_THROWS_NOTHING(Histogram src(Points(0)));
    TS_ASSERT_THROWS_NOTHING(Histogram src(Points{0.1, 0.2, 0.4}));
  }

  void test_construct_from_BinEdges() {
    TS_ASSERT_THROWS_NOTHING(Histogram src(BinEdges(0)));
    TS_ASSERT_THROWS_NOTHING(Histogram src(BinEdges{0.1, 0.2, 0.4}));
  }

  void test_construct_from_invalid_BinEdges() {
    BinEdges binEdges(1);
    TS_ASSERT_THROWS(Histogram{binEdges}, std::logic_error);
  }

  void test_construct_Points_Counts() {
    TS_ASSERT_THROWS_NOTHING(
        Histogram src(Points{0.1, 0.2, 0.4}, Counts{1, 2, 4}));
  }

  void test_construct_Points_Counts_CountVariances() {
    TS_ASSERT_THROWS_NOTHING(
        Histogram src(Points{1.0, 2.0}, Counts(2), CountVariances(2)));
  }

  void test_construct_Points_null_Counts_CountVariances() {
    TS_ASSERT_THROWS_NOTHING(
        Histogram src(Points{1.0, 2.0}, Counts(), CountVariances()));
  }

  void test_construct_values_size_mismatch() {
    TS_ASSERT_THROWS(Histogram(Points(1), Counts(2)), std::logic_error);
    TS_ASSERT_THROWS(Histogram(BinEdges{1.0, 2.0}, Counts(2)),
                     std::logic_error);
    TS_ASSERT_THROWS(Histogram(Points(1), Frequencies(2)), std::logic_error);
    TS_ASSERT_THROWS(Histogram(BinEdges{1.0, 2.0}, Frequencies(2)),
                     std::logic_error);
  }

  void test_construct_values_uncertainties_size_mismatch() {
    TS_ASSERT_THROWS(Histogram(Points{1.0, 2.0}, Counts(2), CountVariances(1)),
                     std::logic_error);
    TS_ASSERT_THROWS(
        Histogram(Points{1.0, 2.0}, Frequencies(2), FrequencyVariances(1)),
        std::logic_error);
  }

  void test_construct_null_values_but_uncertainties_fail() {
    TS_ASSERT_THROWS(Histogram(Points{1.0, 2.0}, Counts(), CountVariances(2)),
                     std::logic_error);
  }

  void test_construct_Counts_automatic_errors() {
    Histogram histogram(BinEdges{1.0, 2.0, 3.0}, Counts{4.0, 9.0});
    TS_ASSERT(histogram.sharedE());
    TS_ASSERT_EQUALS(histogram.e()[0], 2.0);
    TS_ASSERT_EQUALS(histogram.e()[1], 3.0);
  }

  void test_copy_constructor() {
    Histogram src(Points{0.1, 0.2, 0.4});
    Histogram dest(src);
    TS_ASSERT(src.points());
    auto points = dest.points();
    TS_ASSERT(points);
    TS_ASSERT_EQUALS(points.size(), 3);
    TS_ASSERT_EQUALS(points[0], 0.1);
    TS_ASSERT_EQUALS(points[1], 0.2);
    TS_ASSERT_EQUALS(points[2], 0.4);
  }

  void test_move_constructor() {
    Histogram src(Points{0.1, 0.2, 0.4});
    Histogram dest(std::move(src));
    TS_ASSERT(!src.points());
    TS_ASSERT(dest.points());
  }

  void test_copy_assignment() {
    Histogram src(Points{0.1, 0.2, 0.4});
    Histogram dest(Histogram::XMode::BinEdges, Histogram::YMode::Counts);
    TS_ASSERT_EQUALS(dest.xMode(), Histogram::XMode::BinEdges);
    dest = src;
    TS_ASSERT(src.points());
    TS_ASSERT_EQUALS(dest.xMode(), Histogram::XMode::Points);
    auto points = dest.points();
    TS_ASSERT(points);
    TS_ASSERT_EQUALS(points.size(), 3);
    TS_ASSERT_EQUALS(points[0], 0.1);
    TS_ASSERT_EQUALS(points[1], 0.2);
    TS_ASSERT_EQUALS(points[2], 0.4);
  }

  void test_move_assignment() {
    Histogram src(Points{0.1, 0.2, 0.4});
    Histogram dest(Histogram::XMode::BinEdges, Histogram::YMode::Counts);
    TS_ASSERT_EQUALS(dest.xMode(), Histogram::XMode::BinEdges);
    dest = std::move(src);
    TS_ASSERT(!src.points());
    TS_ASSERT(dest.points());
    TS_ASSERT_EQUALS(dest.xMode(), Histogram::XMode::Points);
  }

  void test_xMode() {
    Histogram hist1(Histogram::XMode::Points, Histogram::YMode::Counts);
    TS_ASSERT_EQUALS(hist1.xMode(), Histogram::XMode::Points);
    Histogram hist2(Histogram::XMode::BinEdges, Histogram::YMode::Counts);
    TS_ASSERT_EQUALS(hist2.xMode(), Histogram::XMode::BinEdges);
  }

  void test_getHistogramXMode() {
    TS_ASSERT_EQUALS(getHistogramXMode(0, 0), Histogram::XMode::Points);
    TS_ASSERT_EQUALS(getHistogramXMode(1, 1), Histogram::XMode::Points);
    TS_ASSERT_EQUALS(getHistogramXMode(1, 0), Histogram::XMode::BinEdges);
    TS_ASSERT_EQUALS(getHistogramXMode(2, 1), Histogram::XMode::BinEdges);
    TS_ASSERT_THROWS(getHistogramXMode(2, 0), std::logic_error);
    TS_ASSERT_THROWS(getHistogramXMode(3, 1), std::logic_error);
    TS_ASSERT_THROWS(getHistogramXMode(0, 1), std::logic_error);
  }

  void test_assignment() {
    const Histogram src(Points(1));
    Histogram dest(Points(1));
    TS_ASSERT_THROWS_NOTHING(dest = src);
    TS_ASSERT_EQUALS(&dest.x()[0], &src.x()[0]);
  }

  void test_assignment_mutating() {
    const Histogram src(Points(1));
    Histogram dest(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(dest = src);
    TS_ASSERT_EQUALS(&dest.x()[0], &src.x()[0]);
  }

  void test_assignment_size_change() {
    Histogram src1(Points(1));
    Histogram dest1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(dest1 = src1);
    Histogram src2(Points(1));
    Histogram dest2(BinEdges{1.0, 2.0, 3.0});
    TS_ASSERT_THROWS_NOTHING(dest2 = src2);
    Histogram src3(BinEdges{1.0, 2.0});
    Histogram dest3(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(dest3 = src3);
    Histogram src4(BinEdges{1.0, 2.0});
    Histogram dest4(BinEdges{1.0, 2.0, 3.0});
    TS_ASSERT_THROWS_NOTHING(dest4 = src4);
  }

  void test_points_from_edges() {
    BinEdges binEdges{0.1, 0.2, 0.4};
    const Histogram hist(binEdges);
    const auto points = hist.points();
    TS_ASSERT_DIFFERS(&points[0], &hist.x()[0]);
    TS_ASSERT_EQUALS(points.size(), 2);
    TS_ASSERT_DELTA(points[0], 0.15, 1e-14);
    TS_ASSERT_DELTA(points[1], 0.3, 1e-14);
  }

  void test_points_from_points() {
    const Histogram hist(Points{0.1, 0.2, 0.4});
    const auto points = hist.points();
    TS_ASSERT_EQUALS(&points[0], &hist.x()[0]);
  }

  void test_no_counts_and_frequencies() {
    const Histogram hist(BinEdges{0.1, 0.2, 0.4});
    TS_ASSERT(!hist.counts());
    TS_ASSERT(!hist.countVariances());
    TS_ASSERT(!hist.countStandardDeviations());
    TS_ASSERT(!hist.frequencies());
    TS_ASSERT(!hist.frequencyVariances());
    TS_ASSERT(!hist.frequencyStandardDeviations());
  }

  void test_counts() {
    Histogram hist(BinEdges{0.1, 0.2, 0.4});
    hist.setCounts(Counts{10, 100});
    TS_ASSERT(hist.counts());
    TS_ASSERT_EQUALS(hist.counts().size(), 2);
    TS_ASSERT_EQUALS(hist.counts()[0], 10);
    TS_ASSERT_EQUALS(hist.counts()[1], 100);
  }

  void test_counts_references_internal_data() {
    Histogram hist(BinEdges{0.1, 0.2, 0.4});
    hist.setCounts(Counts{10, 100});
    TS_ASSERT_EQUALS(&hist.counts()[0], &hist.counts()[0]);
  }

  void test_frequencies() {
    Histogram hist(BinEdges{0.1, 0.2, 0.4});
    hist.setCounts(Counts{10, 100});
    TS_ASSERT(hist.frequencies());
    TS_ASSERT_EQUALS(hist.frequencies().size(), 2);
    TS_ASSERT_EQUALS(hist.frequencies()[0], 100.0);
    TS_ASSERT_EQUALS(hist.frequencies()[1], 500.0);
  }

  void test_frequencies_does_not_reference_internal_data() {
    Histogram hist(BinEdges{0.1, 0.2, 0.4});
    hist.setCounts(Counts{10, 100});
    TS_ASSERT_DIFFERS(&hist.frequencies()[0], &hist.frequencies()[0]);
  }

  void test_setPoints_from_vector() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(std::vector<double>{0.1, 0.2}));
    TS_ASSERT_EQUALS(h1.x().size(), 2);
    TS_ASSERT_EQUALS(h1.x()[0], 0.1);
    TS_ASSERT_EQUALS(h1.x()[1], 0.2);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(std::vector<double>{0.1}));
    TS_ASSERT_EQUALS(h2.x().size(), 1);
    TS_ASSERT_EQUALS(h2.x()[0], 0.1);
  }

  void test_setPoints_from_Points() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(Points{0.1, 0.2}));
    TS_ASSERT_EQUALS(h1.x().size(), 2);
    TS_ASSERT_EQUALS(h1.x()[0], 0.1);
    TS_ASSERT_EQUALS(h1.x()[1], 0.2);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(Points{0.1}));
    TS_ASSERT_EQUALS(h2.x().size(), 1);
    TS_ASSERT_EQUALS(h2.x()[0], 0.1);
  }

  void test_setPoints_from_BinEdges() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(BinEdges{0.1, 0.2, 0.4}));
    TS_ASSERT_EQUALS(h1.x().size(), 2);
    TS_ASSERT_DELTA(h1.x()[0], 0.15, 1e-14);
    TS_ASSERT_DELTA(h1.x()[1], 0.3, 1e-14);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(BinEdges{0.1, 0.2}));
    TS_ASSERT_EQUALS(h2.x().size(), 1);
    TS_ASSERT_DELTA(h2.x()[0], 0.15, 1e-14);
  }

  void test_setPoints_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(std::vector<double>(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(Points(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h1.setPoints(BinEdges(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(std::vector<double>(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(Points(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h2.setPoints(BinEdges(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
  }

  void test_setPoints_size_mismatch() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h1.setPoints(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(std::vector<double>{1.0, 2.0, 3.0}),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(Points{1.0, 2.0, 3.0}), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(BinEdges{1.0, 2.0}), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(BinEdges{1.0, 2.0, 3.0, 4.0}),
                     std::logic_error);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS(h2.setPoints(std::vector<double>(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(std::vector<double>{1.0, 2.0}),
                     std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(Points(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(Points{1.0, 2.0}), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(BinEdges(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(BinEdges{1.0, 2.0, 3.0}), std::logic_error);
  }

  void test_setPoints_size_mismatch_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS(h1.setPoints(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setPoints(BinEdges(1)), std::logic_error);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS(h2.setPoints(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setPoints(BinEdges(1)), std::logic_error);
  }

  void test_setPoints_self_assignment() {
    Histogram h(Points(0));
    auto &x = h.x();
    auto old_address = &x;
    h.setPoints(x);
    TS_ASSERT_EQUALS(&h.x(), old_address);
  }

  void test_setPoints_legacy_self_assignment() {
    Histogram h(Points(0));
    auto &x = h.readX();
    auto old_address = &x;
    h.setPoints(x);
    TS_ASSERT_EQUALS(&h.readX(), old_address);
  }

  void test_setPoints_self_assignment_with_size_mismatch() {
    Histogram h(BinEdges{1.0, 2.0});
    auto &x = h.x();
    // This test makes sure that size mismatch takes precedence over the
    // self-assignment check. x is bin edges, setting it as points should fail.
    TS_ASSERT_THROWS(h.setPoints(x), std::logic_error);
  }

  void test_setPoints_changesDxStorageMode() {
    Histogram hist(BinEdges{1.0, 2.0, 3.0});
    auto dx = {1.0, 2.0, 3.0};
    hist.setBinEdgeStandardDeviations(dx);
    TS_ASSERT_EQUALS(hist.dx().size(), 3);
    hist.setPoints(Points{1.0, 2.0});
    TS_ASSERT_EQUALS(hist.dx().size(), 2);
    TS_ASSERT_DELTA(hist.dx()[0], 1.5, 1e-14);
    TS_ASSERT_DELTA(hist.dx()[1], 2.5, 1e-14);
  }

  void test_edges_from_edges() {
    const Histogram hist(BinEdges{0.1, 0.2, 0.4});
    const auto edges = hist.binEdges();
    TS_ASSERT_EQUALS(&edges[0], &hist.x()[0]);
    TS_ASSERT_EQUALS(edges.size(), 3);
  }

  void test_edges_from_points() {
    const Histogram hist(Points{0.1, 0.2, 0.4});
    const auto edges = hist.binEdges();
    TS_ASSERT_DIFFERS(&edges[0], &hist.x()[0]);
    TS_ASSERT_EQUALS(edges.size(), 4);
  }

  void test_setBinEdges() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(
        h1.setBinEdges(std::vector<double>{1.0, 2.0, 3.0}));
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(Points{1.0, 2.0}));
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(BinEdges{1.0, 2.0, 3.0}));
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(std::vector<double>{1.0, 2.0}));
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(Points(1)));
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(BinEdges{1.0, 2.0}));
  }

  void test_setBinEdges_from_vector() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(
        h1.setBinEdges(std::vector<double>{0.1, 0.2, 0.4}));
    TS_ASSERT_EQUALS(h1.x().size(), 3);
    TS_ASSERT_EQUALS(h1.x()[0], 0.1);
    TS_ASSERT_EQUALS(h1.x()[1], 0.2);
    TS_ASSERT_EQUALS(h1.x()[2], 0.4);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(std::vector<double>{0.1, 0.2}));
    TS_ASSERT_EQUALS(h2.x().size(), 2);
    TS_ASSERT_EQUALS(h2.x()[0], 0.1);
    TS_ASSERT_EQUALS(h2.x()[1], 0.2);
  }

  void test_setBinEdges_from_Points() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(Points{0.1, 0.3}));
    TS_ASSERT_EQUALS(h1.x().size(), 3);
    TS_ASSERT_DELTA(h1.x()[0], 0.0, 1e-14);
    TS_ASSERT_DELTA(h1.x()[1], 0.2, 1e-14);
    TS_ASSERT_DELTA(h1.x()[2], 0.4, 1e-14);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(Points{1.0}));
    TS_ASSERT_EQUALS(h2.x().size(), 2);
    TS_ASSERT_DELTA(h2.x()[0], 0.5, 1e-14);
    TS_ASSERT_DELTA(h2.x()[1], 1.5, 1e-14);
  }

  void test_setBinEdges_from_BinEdges() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(BinEdges{0.1, 0.2, 0.4}));
    TS_ASSERT_EQUALS(h1.x().size(), 3);
    TS_ASSERT_EQUALS(h1.x()[0], 0.1);
    TS_ASSERT_EQUALS(h1.x()[1], 0.2);
    TS_ASSERT_EQUALS(h1.x()[2], 0.4);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(BinEdges{0.1, 0.2}));
    TS_ASSERT_EQUALS(h2.x().size(), 2);
    TS_ASSERT_EQUALS(h2.x()[0], 0.1);
    TS_ASSERT_EQUALS(h2.x()[1], 0.2);
  }

  void test_setBinEdges_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(std::vector<double>(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(Points(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h1.setBinEdges(BinEdges(0)));
    TS_ASSERT_EQUALS(h1.x().size(), 0);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(std::vector<double>(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(Points(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
    TS_ASSERT_THROWS_NOTHING(h2.setBinEdges(BinEdges(0)));
    TS_ASSERT_EQUALS(h2.x().size(), 0);
  }

  void test_setBinEdges_size_mismatch() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h1.setBinEdges(std::vector<double>{1.0, 2.0}),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(std::vector<double>{1.0, 2.0, 3.0, 4.0}),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(Points{1.0, 2.0, 3.0}), std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(BinEdges{1.0, 2.0}), std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(BinEdges{1.0, 2.0, 3.0, 4.0}),
                     std::logic_error);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS(h2.setBinEdges(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(std::vector<double>{1.0, 2.0, 3.0}),
                     std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(Points(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(Points{1.0, 2.0}), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(BinEdges(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(BinEdges{1.0, 2.0, 3.0}), std::logic_error);
  }

  void test_setBinEdges_size_mismatch_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS(h1.setBinEdges(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setBinEdges(BinEdges(1)), std::logic_error);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS(h2.setBinEdges(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(Points(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setBinEdges(BinEdges(1)), std::logic_error);
  }

  void test_setBinEdges_self_assignment() {
    Histogram h(BinEdges(0));
    auto &x = h.x();
    auto old_address = &x;
    h.setBinEdges(x);
    TS_ASSERT_EQUALS(&h.x(), old_address);
  }

  void test_setBinEdges_legacy_self_assignment() {
    Histogram h(BinEdges(0));
    auto &x = h.readX();
    auto old_address = &x;
    h.setBinEdges(x);
    TS_ASSERT_EQUALS(&h.readX(), old_address);
  }

  void test_setBinEdges_self_assignment_with_size_mismatch() {
    Histogram h(Points{1.0, 2.0});
    auto &x = h.x();
    // This test makes sure that size mismatch takes precedence over the
    // self-assignment check. x is points, setting it as bin edges should fail.
    TS_ASSERT_THROWS(h.setBinEdges(x), std::logic_error);
  }

  void test_setBinEdges_changesDxStorageMode() {
    Histogram hist(Points{1.0, 2.0});
    auto dx = {1.0, 2.0};
    hist.setPointStandardDeviations(dx);
    TS_ASSERT_EQUALS(hist.dx().size(), 2);
    hist.setBinEdges(BinEdges{1.0, 2.0, 3.0});
    TS_ASSERT_EQUALS(hist.dx().size(), 3);
    TS_ASSERT_DELTA(hist.dx()[0], 0.5, 1e-14);
    TS_ASSERT_DELTA(hist.dx()[1], 1.5, 1e-14);
    TS_ASSERT_DELTA(hist.dx()[2], 2.5, 1e-14);
  }

  void test_setCounts_size_mismatch() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h1.setCounts(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setCounts(std::vector<double>(3)), std::logic_error);
    TS_ASSERT_THROWS(h1.setCounts(Counts(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setCounts(Counts(3)), std::logic_error);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS(h2.setCounts(std::vector<double>(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setCounts(std::vector<double>(2)), std::logic_error);
    TS_ASSERT_THROWS(h2.setCounts(Counts(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setCounts(Counts(2)), std::logic_error);
  }

  void test_setCounts_size_mismatch_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS(h1.setCounts(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setCounts(Counts(1)), std::logic_error);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS(h2.setCounts(std::vector<double>(1)), std::logic_error);
    TS_ASSERT_THROWS(h2.setCounts(Counts(1)), std::logic_error);
  }

  void test_setCounts_self_assignment() {
    Histogram h(Points(0));
    h.setCounts(0);
    auto &y = h.y();
    auto old_address = &y;
    h.setCounts(y);
    TS_ASSERT_EQUALS(&h.y(), old_address);
  }

  void test_setCounts_legacy_self_assignment() {
    Histogram h(Points(0));
    h.setCounts(0);
    auto &y = h.readY();
    auto old_address = &y;
    h.setPoints(y);
    TS_ASSERT_EQUALS(&h.readY(), old_address);
  }

  void test_setFrequencies_size_mismatch() {
    Histogram h1(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h1.setFrequencies(std::vector<double>(1)),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setFrequencies(std::vector<double>(3)),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setFrequencies(Frequencies(1)), std::logic_error);
    TS_ASSERT_THROWS(h1.setFrequencies(Frequencies(3)), std::logic_error);
    Histogram h2(BinEdges{1.0, 2.0});
    TS_ASSERT_THROWS(h2.setFrequencies(std::vector<double>(0)),
                     std::logic_error);
    TS_ASSERT_THROWS(h2.setFrequencies(std::vector<double>(2)),
                     std::logic_error);
    TS_ASSERT_THROWS(h2.setFrequencies(Frequencies(0)), std::logic_error);
    TS_ASSERT_THROWS(h2.setFrequencies(Frequencies(2)), std::logic_error);
  }

  void test_setFrequencies_size_mismatch_degenerate() {
    Histogram h1(Points(0));
    TS_ASSERT_THROWS(h1.setFrequencies(std::vector<double>(1)),
                     std::logic_error);
    TS_ASSERT_THROWS(h1.setFrequencies(Frequencies(1)), std::logic_error);
    Histogram h2(BinEdges(0));
    TS_ASSERT_THROWS(h2.setFrequencies(std::vector<double>(1)),
                     std::logic_error);
    TS_ASSERT_THROWS(h2.setFrequencies(Frequencies(1)), std::logic_error);
  }

  void test_setFrequencies_self_assignment() {
    Histogram h(Points(0));
    h.setFrequencies(0);
    auto &y = h.y();
    auto old_address = &y;
    h.setFrequencies(y);
    TS_ASSERT_EQUALS(&h.y(), old_address);
  }

  void test_setFrequencies_legacy_self_assignment() {
    Histogram h(Points(0));
    h.setFrequencies(0);
    auto &y = h.readY();
    auto old_address = &y;
    h.setFrequencies(y);
    TS_ASSERT_EQUALS(&h.readY(), old_address);
  }

  void test_setCountVariances() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h.setCountVariances(2));
  }

  void test_setCountStandardDeviations() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h.setCountStandardDeviations(2));
  }

  void test_setFrequencyVariances() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h.setFrequencyVariances(2));
  }

  void test_setFrequencyStandardDeviations() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS_NOTHING(h.setFrequencyStandardDeviations(2));
  }

  void test_setCountVariances_size_mismatch() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h.setCountVariances(1), std::logic_error);
    TS_ASSERT_THROWS(h.setCountVariances(3), std::logic_error);
  }

  void test_setCountStandardDeviations_size_mismatch() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h.setCountStandardDeviations(1), std::logic_error);
    TS_ASSERT_THROWS(h.setCountStandardDeviations(3), std::logic_error);
  }

  void test_setFrequencyVariances_size_mismatch() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h.setFrequencyVariances(1), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyVariances(3), std::logic_error);
  }

  void test_setFrequencyStandardDeviations_size_mismatch() {
    Histogram h(Points{1.0, 2.0});
    TS_ASSERT_THROWS(h.setFrequencyStandardDeviations(1), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyStandardDeviations(3), std::logic_error);
  }

  void test_error_setter_self_assignment() {
    Histogram h(Points{1.0, 2.0});
    h.setCountVariances(2);
    auto &e = h.e();
    auto old_address = &e;
    // e is always count standard deviations, self assignment as variances or
    // frequencies should fail unless we are setting count standard deviations.
    TS_ASSERT_THROWS(h.setCountVariances(e), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyVariances(e), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyStandardDeviations(e), std::logic_error);
    TS_ASSERT_THROWS_NOTHING(h.setCountStandardDeviations(e));
    TS_ASSERT_EQUALS(&h.e(), old_address);
  }

  void test_x() {
    Histogram hist(Points({0.1, 0.2, 0.4}));
    TS_ASSERT_EQUALS(hist.x()[0], 0.1);
    TS_ASSERT_EQUALS(hist.x()[1], 0.2);
    TS_ASSERT_EQUALS(hist.x()[2], 0.4);
  }

  void test_x_references_internal_data() {
    Histogram hist(Points(0));
    auto copy(hist);
    TS_ASSERT_EQUALS(&hist.x(), &copy.x());
  }

  void test_mutableX() {
    Histogram hist(Points({0.1, 0.2, 0.4}));
    TS_ASSERT_EQUALS(hist.mutableX()[0], 0.1);
    TS_ASSERT_EQUALS(hist.mutableX()[1], 0.2);
    TS_ASSERT_EQUALS(hist.mutableX()[2], 0.4);
  }

  void test_mutableX_triggers_copy() {
    Histogram hist(Points(0));
    auto copy(hist);
    TS_ASSERT_DIFFERS(&hist.x(), &copy.mutableX());
  }

  void test_x_references_same_data_as_binEdges() {
    Histogram hist(BinEdges(0));
    TS_ASSERT_EQUALS(&hist.x(), &hist.binEdges().data());
    TS_ASSERT_DIFFERS(&hist.x(), &hist.points().data());
  }

  void test_x_references_same_data_as_points() {
    Histogram hist(Points(0));
    TS_ASSERT_DIFFERS(&hist.x(), &hist.binEdges().data());
    TS_ASSERT_EQUALS(&hist.x(), &hist.points().data());
  }

  void test_sharedX() {
    auto data = Mantid::Kernel::make_cow<HistogramX>(0);
    Histogram hist{BinEdges(data)};
    TS_ASSERT_EQUALS(hist.sharedX(), data);
  }

  void test_setSharedX() {
    auto data1 = Mantid::Kernel::make_cow<HistogramX>(0);
    auto data2 = Mantid::Kernel::make_cow<HistogramX>(0);
    Histogram hist{BinEdges(data1)};
    TS_ASSERT_EQUALS(hist.sharedX(), data1);
    TS_ASSERT_THROWS_NOTHING(hist.setSharedX(data2));
    TS_ASSERT_DIFFERS(hist.sharedX(), data1);
    TS_ASSERT_EQUALS(hist.sharedX(), data2);
  }

  void test_setSharedX_size_mismatch() {
    auto data1 = Mantid::Kernel::make_cow<HistogramX>(0);
    auto tmp = {1.0, 2.0};
    auto data2 = Mantid::Kernel::make_cow<HistogramX>(tmp);
    Histogram hist{BinEdges(data1)};
    TS_ASSERT_THROWS(hist.setSharedX(data2), std::logic_error);
  }

  void test_setSharedX_catches_misuse() {
    BinEdges edges{1.0, 2.0};
    Histogram hist(edges);
    auto points = hist.points();
    TS_ASSERT_THROWS(hist.setSharedX(points.cowData()), std::logic_error);
  }

  void test_y() {
    Histogram hist(Points{1.0, 2.0, 3.0});
    hist.setCounts(Counts{0.1, 0.2, 0.4});
    TS_ASSERT_EQUALS(hist.y()[0], 0.1);
    TS_ASSERT_EQUALS(hist.y()[1], 0.2);
    TS_ASSERT_EQUALS(hist.y()[2], 0.4);
  }

  void test_y_references_internal_data() {
    Histogram hist(Points(0));
    hist.setCounts(0);
    auto copy(hist);
    TS_ASSERT_EQUALS(&hist.y(), &copy.y());
  }

  void test_mutableY() {
    Histogram hist(Points{1.0, 2.0, 3.0});
    hist.setCounts(Counts{0.1, 0.2, 0.4});
    TS_ASSERT_EQUALS(hist.mutableY()[0], 0.1);
    TS_ASSERT_EQUALS(hist.mutableY()[1], 0.2);
    TS_ASSERT_EQUALS(hist.mutableY()[2], 0.4);
  }

  void test_mutableY_triggers_copy() {
    Histogram hist(Points(0));
    hist.setCounts(0);
    auto copy(hist);
    TS_ASSERT_DIFFERS(&hist.y(), &copy.mutableY());
  }

  void test_y_references_same_data_as_counts() {
    Histogram hist(Points(0));
    hist.setCounts(0);
    TS_ASSERT_EQUALS(&hist.y(), &hist.counts().data());
    TS_ASSERT_DIFFERS(&hist.y(), &hist.frequencies().data());
  }

  void test_sharedY() {
    auto data = Mantid::Kernel::make_cow<HistogramY>(0);
    Histogram hist{BinEdges(0)};
    hist.setCounts(data);
    TS_ASSERT_EQUALS(hist.sharedY(), data);
  }

  void test_setSharedY() {
    auto data1 = Mantid::Kernel::make_cow<HistogramY>(0);
    auto data2 = Mantid::Kernel::make_cow<HistogramY>(0);
    Histogram hist{BinEdges(0)};
    hist.setCounts(data1);
    TS_ASSERT_EQUALS(hist.sharedY(), data1);
    TS_ASSERT_THROWS_NOTHING(hist.setSharedY(data2));
    TS_ASSERT_DIFFERS(hist.sharedY(), data1);
    TS_ASSERT_EQUALS(hist.sharedY(), data2);
  }

  void test_setSharedY_size_mismatch() {
    auto data1 = Mantid::Kernel::make_cow<HistogramY>(0);
    auto data2 = Mantid::Kernel::make_cow<HistogramY>(2);
    Histogram hist{BinEdges(0)};
    hist.setCounts(data1);
    TS_ASSERT_THROWS(hist.setSharedY(data2), std::logic_error);
  }

  void test_e() {
    Histogram hist(Points{1.0, 2.0, 3.0});
    hist.setCountStandardDeviations(CountStandardDeviations{0.1, 0.2, 0.4});
    TS_ASSERT_EQUALS(hist.e()[0], 0.1);
    TS_ASSERT_EQUALS(hist.e()[1], 0.2);
    TS_ASSERT_EQUALS(hist.e()[2], 0.4);
  }

  void test_e_references_internal_data() {
    Histogram hist(Points(0));
    hist.setCountStandardDeviations(0);
    auto copy(hist);
    TS_ASSERT_EQUALS(&hist.e(), &copy.e());
  }

  void test_mutableE() {
    Histogram hist(Points{1.0, 2.0, 3.0});
    hist.setCountStandardDeviations(CountStandardDeviations{0.1, 0.2, 0.4});
    TS_ASSERT_EQUALS(hist.mutableE()[0], 0.1);
    TS_ASSERT_EQUALS(hist.mutableE()[1], 0.2);
    TS_ASSERT_EQUALS(hist.mutableE()[2], 0.4);
  }

  void test_mutableE_triggers_copy() {
    Histogram hist(Points(0));
    hist.setCountStandardDeviations(0);
    auto copy(hist);
    TS_ASSERT_DIFFERS(&hist.e(), &copy.mutableE());
  }

  void test_e_references_same_data_as_counts() {
    Histogram hist(Points(0));
    hist.setCountStandardDeviations(0);
    TS_ASSERT_EQUALS(&hist.e(), &hist.countStandardDeviations().data());
    TS_ASSERT_DIFFERS(&hist.e(), &hist.frequencyStandardDeviations().data());
  }

  void test_sharedE() {
    auto data = Mantid::Kernel::make_cow<HistogramE>(0);
    Histogram hist{BinEdges(0)};
    hist.setCountStandardDeviations(data);
    TS_ASSERT_EQUALS(hist.sharedE(), data);
  }

  void test_setSharedE() {
    auto data1 = Mantid::Kernel::make_cow<HistogramE>(0);
    auto data2 = Mantid::Kernel::make_cow<HistogramE>(0);
    Histogram hist{BinEdges(0)};
    hist.setCountStandardDeviations(data1);
    TS_ASSERT_EQUALS(hist.sharedE(), data1);
    TS_ASSERT_THROWS_NOTHING(hist.setSharedE(data2));
    TS_ASSERT_DIFFERS(hist.sharedE(), data1);
    TS_ASSERT_EQUALS(hist.sharedE(), data2);
  }

  void test_setSharedE_size_mismatch() {
    auto data1 = Mantid::Kernel::make_cow<HistogramE>(0);
    auto data2 = Mantid::Kernel::make_cow<HistogramE>(2);
    Histogram hist{BinEdges(0)};
    hist.setCountStandardDeviations(data1);
    TS_ASSERT_THROWS(hist.setSharedE(data2), std::logic_error);
  }

  void test_yMode() {
    Histogram hist1(Histogram::XMode::Points, Histogram::YMode::Counts);
    TS_ASSERT_EQUALS(hist1.yMode(), Histogram::YMode::Counts);
    Histogram hist2(Histogram::XMode::Points, Histogram::YMode::Frequencies);
    TS_ASSERT_EQUALS(hist2.yMode(), Histogram::YMode::Frequencies);
  }

  void test_yMode_Uninitialized() {
    Histogram hist(Points(1));
    TS_ASSERT_EQUALS(hist.yMode(), Histogram::YMode::Uninitialized);
  }

  void test_yMode_initialized_by_setCounts() {
    Histogram h(Points(2));
    h.setCounts(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Counts);
  }

  void test_yMode_initialized_by_setCountStandardDeviations() {
    Histogram h(Points(2));
    h.setCountStandardDeviations(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Counts);
  }

  void test_yMode_initialized_by_setCountVariances() {
    Histogram h(Points(2));
    h.setCountVariances(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Counts);
  }

  void test_yMode_initialized_by_setFrequencies() {
    Histogram h(Points(2));
    h.setFrequencies(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Frequencies);
  }

  void test_yMode_initialized_by_setFrequencyStandardDeviations() {
    Histogram h(Points(2));
    h.setFrequencyStandardDeviations(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Frequencies);
  }

  void test_yMode_initialized_by_setFrequencyVariances() {
    Histogram h(Points(2));
    h.setFrequencyVariances(2);
    TS_ASSERT_EQUALS(h.yMode(), Histogram::YMode::Frequencies);
  }

  void test_yMode_cannot_be_changed_by_Count_setters() {
    Histogram h(Points(1), Counts(1));
    TS_ASSERT_THROWS(h.setFrequencies(1), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyVariances(1), std::logic_error);
    TS_ASSERT_THROWS(h.setFrequencyStandardDeviations(1), std::logic_error);
  }

  void test_yMode_cannot_be_changed_by_Frequency_setters() {
    Histogram h(Points(1), Frequencies(1));
    TS_ASSERT_THROWS(h.setCounts(1), std::logic_error);
    TS_ASSERT_THROWS(h.setCountVariances(1), std::logic_error);
    TS_ASSERT_THROWS(h.setCountStandardDeviations(1), std::logic_error);
  }

  void test_setSharedY_fails_for_YMode_Uninitialized() {
    Histogram hist(Points(1));
    Counts counts(1);
    TS_ASSERT_THROWS(hist.setSharedY(counts.cowData()), std::logic_error);
  }
};

class HistogramTestPerformance : public CxxTest::TestSuite {
public:
  static HistogramTestPerformance *createSuite() {
    return new HistogramTestPerformance;
  }
  static void destroySuite(HistogramTestPerformance *suite) { delete suite; }

  HistogramTestPerformance() : copy(histSize, LinearGenerator(0, 2)) {
    for (size_t i = 0; i < nHists; i++)
      hists.push_back(Histogram(BinEdges(histSize, LinearGenerator(0, 1))));

    histsCopy = hists;
  }

  void test_copy_many_X() {
    for (auto &i : hists)
      i.mutableX() = copy;
  }

  void test_share_many_X() {
    hists[0].mutableX() = copy;
    for (auto &i : hists)
      i.setSharedX(hists[0].sharedX());
  }

private:
  const size_t nHists = 100'000;
  const size_t histSize = 10'000;
  std::vector<Histogram> hists;
  std::vector<Histogram> histsCopy;
  HistogramX copy;
};

#endif /* MANTID_HISTOGRAMDATA_HISTOGRAMTEST_H_ */
