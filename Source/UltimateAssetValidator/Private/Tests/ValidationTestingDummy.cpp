// Copyright 2022 Tadeáš Anděl, All Rights Reserved.


#include "Tests/ValidationTestingDummy.h"

AValidationTestingDummy::AValidationTestingDummy()
{
	DummyTester = CreateDefaultSubobject<UValidationTestingDummyComponent>(TEXT("Dummy tester"));
}
