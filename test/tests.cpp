// Copyright 2021 GHA Test Team
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "../include/TimedDoor.h"

// Mock TimerClient for testing
class MockTimerClient : public TimerClient {
 public:
    MOCK_METHOD(void, Timeout, (), (override));
};

// Test fixture for TimedDoor
class TimedDoorTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(2);
        adapter = new DoorTimerAdapter(*door);
    }

    void TearDown() override {
        delete door;
        delete adapter;
    }

    TimedDoor* door;
    DoorTimerAdapter* adapter;
};

// Test fixture for Timer
class TimerTest : public ::testing::Test {
 protected:
    void SetUp() override {
        timer = new Timer();
        mockClient = new MockTimerClient();
    }

    void TearDown() override {
        delete timer;
        delete mockClient;
    }

    Timer* timer;
    MockTimerClient* mockClient;
};

// Tests for TimedDoor
TEST_F(TimedDoorTest, DoorInitiallyClosed) {
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, UnlockOpensDoor) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, LockClosesDoor) {
    door->unlock();
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, GetTimeOutReturnsCorrectValue) {
    EXPECT_EQ(door->getTimeOut(), 2);
}

TEST_F(TimedDoorTest, ThrowStateThrowsException) {
    EXPECT_THROW(door->throwState(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutWhenDoorOpenThrows) {
    door->unlock();
    EXPECT_THROW(adapter->Timeout(), std::runtime_error);
}

TEST_F(TimedDoorTest, AdapterTimeoutWhenDoorClosedNoThrow) {
    door->lock();
    EXPECT_NO_THROW(adapter->Timeout());
}

TEST_F(TimedDoorTest, MultipleUnlockLockOperations) {
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
    door->unlock();
    EXPECT_TRUE(door->isDoorOpened());
    door->lock();
    EXPECT_FALSE(door->isDoorOpened());
}

// Integration tests with Timer
class TimedDoorIntegrationTest : public ::testing::Test {
 protected:
    void SetUp() override {
        door = new TimedDoor(1);
        timer = new Timer();
    }

    void TearDown() override {
        delete door;
        delete timer;
    }

    TimedDoor* door;
    Timer* timer;
};

TEST_F(TimedDoorIntegrationTest, TimerTriggersTimeoutOnOpenDoor) {
    door->unlock();
    DoorTimerAdapter adapter(*door);

    EXPECT_THROW({
        timer->tregister(door->getTimeOut(), &adapter);
    }, std::runtime_error);
}

TEST_F(TimedDoorIntegrationTest, TimerDoesNotTriggerOnClosedDoor) {
    door->lock();
    DoorTimerAdapter adapter(*door);

    EXPECT_NO_THROW({
        timer->tregister(door->getTimeOut(), &adapter);
    });
}

TEST_F(TimedDoorIntegrationTest, DoorStateChangesAfterTimeout) {
    door->unlock();
    DoorTimerAdapter adapter(*door);

    try {
        timer->tregister(door->getTimeOut(), &adapter);
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "Door has been open for too long!");
    }
}

// Tests with MockTimerClient
TEST_F(TimerTest, TregisterCallsTimeoutOnClient) {
    EXPECT_CALL(*mockClient, Timeout()).Times(1);

    std::thread t([this]() {
        timer->tregister(0, mockClient);
    });
    t.join();
}

TEST_F(TimerTest, TregisterHandlesNullClient) {
    EXPECT_NO_THROW({
        timer->tregister(1, nullptr);
    });
}

// Additional tests for different timeout values
TEST_F(TimedDoorTest, DifferentTimeoutValues) {
    TimedDoor shortDoor(1);
    TimedDoor longDoor(5);

    EXPECT_EQ(shortDoor.getTimeOut(), 1);
    EXPECT_EQ(longDoor.getTimeOut(), 5);

    shortDoor.unlock();
    longDoor.unlock();

    DoorTimerAdapter shortAdapter(shortDoor);
    DoorTimerAdapter longAdapter(longDoor);

    EXPECT_THROW(shortAdapter.Timeout(), std::runtime_error);
    EXPECT_THROW(longAdapter.Timeout(), std::runtime_error);
}

// Test multiple adapters same door
TEST_F(TimedDoorTest, MultipleAdaptersSameDoor) {
    DoorTimerAdapter adapter1(*door);
    DoorTimerAdapter adapter2(*door);

    door->unlock();

    EXPECT_THROW(adapter1.Timeout(), std::runtime_error);
    EXPECT_THROW(adapter2.Timeout(), std::runtime_error);
}

// Test door operations after timeout
TEST_F(TimedDoorIntegrationTest, DoorCanBeClosedAfterTimeout) {
    door->unlock();
    DoorTimerAdapter adapter(*door);

    try {
        timer->tregister(door->getTimeOut(), &adapter);
    } catch (const std::runtime_error&) {
        door->lock();
        EXPECT_FALSE(door->isDoorOpened());
    }
}
