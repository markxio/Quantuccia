/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2016 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_instruments_hpp
#define quantlib_test_instruments_hpp

#include <boost/test/unit_test.hpp>

/* remember to document new and/or updated tests in the Doxygen
   comment block of the corresponding class */

class InstrumentTest {
  public:
    static void testObservable();
    static void testCompositeWhenShiftingDates();
    static boost::unit_test_framework::test_suite* suite();
};


/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "utilities.hpp"
#include <ql/instruments/stock.hpp>
#include <ql/instruments/compositeinstrument.hpp>
#include <ql/instruments/europeanoption.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/daycounters/actual360.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

void InstrumentTest::testObservable() {

    BOOST_TEST_MESSAGE("Testing observability of instruments...");

    boost::shared_ptr<SimpleQuote> me1(new SimpleQuote(0.0));
    RelinkableHandle<Quote> h(me1);
    boost::shared_ptr<Instrument> s(new Stock(h));

    Flag f;
    f.registerWith(s);
    
    s->NPV();
    me1->setValue(3.14);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");
    
    s->NPV();
    f.lower();
    boost::shared_ptr<SimpleQuote> me2(new SimpleQuote(0.0));
    h.linkTo(me2);
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");

    f.lower();
    s->freeze();
    s->NPV();
    me2->setValue(2.71);
    if (f.isUp())
        BOOST_FAIL("Observer was notified of frozen instrument change");
    s->NPV();
    s->unfreeze();
    if (!f.isUp())
        BOOST_FAIL("Observer was not notified of instrument change");
}


void InstrumentTest::testCompositeWhenShiftingDates() {

    BOOST_TEST_MESSAGE(
        "Testing reaction of composite instrument to date changes...");

    SavedSettings backup;

    Date today = Date::todaysDate();
    DayCounter dc = Actual360();

    boost::shared_ptr<StrikedTypePayoff> payoff(
                                 new PlainVanillaPayoff(Option::Call, 100.0));
    boost::shared_ptr<Exercise> exercise(new EuropeanExercise(today+30));

    boost::shared_ptr<Instrument> option(new EuropeanOption(payoff, exercise));

    boost::shared_ptr<SimpleQuote> spot(new SimpleQuote(100.0));
    boost::shared_ptr<YieldTermStructure> qTS = flatRate(0.0, dc);
    boost::shared_ptr<YieldTermStructure> rTS = flatRate(0.01, dc);
    boost::shared_ptr<BlackVolTermStructure> volTS = flatVol(0.1, dc);

    boost::shared_ptr<BlackScholesMertonProcess> process(
        new BlackScholesMertonProcess(Handle<Quote>(spot),
                                      Handle<YieldTermStructure>(qTS),
                                      Handle<YieldTermStructure>(rTS),
                                      Handle<BlackVolTermStructure>(volTS)));
    boost::shared_ptr<PricingEngine> engine(new AnalyticEuropeanEngine(process));

    option->setPricingEngine(engine);

    CompositeInstrument composite;
    composite.add(option);

    Settings::instance().evaluationDate() = today+45;

    if (!composite.isExpired())
        BOOST_FAIL("Composite didn't detect expiration");
    if (composite.NPV() != 0.0)
        BOOST_FAIL("Composite didn't return a null NPV");

    Settings::instance().evaluationDate() = today;

    if (composite.isExpired())
        BOOST_FAIL("Composite didn't detect aliveness");
    if (composite.NPV() == 0.0)
        BOOST_FAIL("Composite didn't recalculate");
}

test_suite* InstrumentTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Instrument tests");
    suite->add(QUANTLIB_TEST_CASE(&InstrumentTest::testObservable));
    suite->add(QUANTLIB_TEST_CASE(
                            &InstrumentTest::testCompositeWhenShiftingDates));
    return suite;
}



#endif