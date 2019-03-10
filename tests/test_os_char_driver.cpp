/**
 * @file	test_os_char_driver.cpp
 * @brief	Tests os_char_driver template
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */
#include "os_char_driver.hpp"
#include "os_task.hpp"
#include "unity.h"
#include <array>
#include <csignal>
#include <functional>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

using namespace jungles;
#define SIGNAL_TX SIGRTMIN
#define SIGNAL_RX (SIGRTMIN + 1)

// --------------------------------------------------------------------------------------------------------------------
// DECLARATION OF THE TEST CASES
// --------------------------------------------------------------------------------------------------------------------
static void UNIT_TEST_1_block_on_read_and_unblock_on_message_received();
static void UNIT_TEST_2_blocking_write_multiple_string_types();

// --------------------------------------------------------------------------------------------------------------------
// DECLARATION OF PRIVATE FUNCTIONS AND VARIABLES
// --------------------------------------------------------------------------------------------------------------------
static std::function<void(void)> tx_isr_handler, rx_isr_handler;
static std::function<void(char)> byte_sender;
static void helper_set_tx_isr_handler(std::function<void(void)> f);
static void helper_set_rx_isr_handler(std::function<void(void)> f);
static void helper_set_byte_sender(std::function<void(char)> f);

static bool tx_isr_enabled;
static void tx_isr_handler_callback(int signal);
static void rx_isr_handler_callback(int signal);
static void tx_it_enable();
static void tx_it_disable();
static void rx_it_enable();
static void rx_it_disable();
static void byte_send(char c);

// --------------------------------------------------------------------------------------------------------------------
// EXECUTION OF THE TESTS
// --------------------------------------------------------------------------------------------------------------------
void test_os_char_driver()
{
    std::signal(SIGNAL_TX, tx_isr_handler_callback);
    std::signal(SIGNAL_RX, rx_isr_handler_callback);

    RUN_TEST(UNIT_TEST_1_block_on_read_and_unblock_on_message_received);
    RUN_TEST(UNIT_TEST_2_blocking_write_multiple_string_types);

    std::signal(SIGNAL_TX, SIG_DFL);
    std::signal(SIGNAL_RX, SIG_DFL);
}

// --------------------------------------------------------------------------------------------------------------------
// DEFINITION OF THE TEST CASES
// --------------------------------------------------------------------------------------------------------------------
static void UNIT_TEST_1_block_on_read_and_unblock_on_message_received()
{
    os_char_driver<64, 16> chardrv{tx_it_enable, tx_it_disable, rx_it_enable, rx_it_disable, byte_send};

    auto reader_task_handle = os_task_get_current_task_handle();
    os_task sync_reader_task(
        [reader_task_handle, &chardrv]() {
            // Ensure readline() will be called
            os_task_yield();
            os_task_yield();
            os_task_yield();
            for (unsigned state = os_task_state_running;
                 state != os_task_state_blocked && state != os_task_state_blocked;
                 state = os_task_get_state(reader_task_handle))
                os_delay_ms(1);
            helper_set_rx_isr_handler([&chardrv]() {
                static constexpr char test_string_rcvd[] = "makapaka";
                static const char *it = test_string_rcvd;
                static constexpr const char *end =
                    test_string_rcvd + std::distance(std::begin(test_string_rcvd), std::end(test_string_rcvd));
                if (it != end)
                {
                    chardrv.rx_isr_handler(*it++);
                    std::raise(SIGNAL_RX);
                }
            });
            std::raise(SIGNAL_RX);
        },
        "sync_reader",
        256,
        1);

    auto line = chardrv.readline(os_no_timeout);

    TEST_ASSERT_EQUAL_STRING("makapaka", line.c_str());
    os_task_yield();
}

static void UNIT_TEST_2_blocking_write_multiple_string_types()
{
    os_char_driver<64, 16> chardrv{tx_it_enable, tx_it_disable, rx_it_enable, rx_it_disable, byte_send};
    std::string s{"std::string"};
    std::vector v{'s', 't', 'd', ':', ':', 'v', 'e', 'c', 't', 'o', 'r'};
    std::array<char, 10> a{'s', 't', 'd', ':', ':', 'a', 'r', 'r', 'a', 'y'};
    std::string sv{"std::string_view"};

    std::string result;
    helper_set_byte_sender([&result](char c) { result += c; });
    helper_set_tx_isr_handler([&chardrv]() { chardrv.tx_isr_handler(); });
    chardrv.write(s, v, a, sv);

    TEST_ASSERT_EQUAL_STRING("std::stringstd::vectorstd::arraystd::string_view", result.c_str());
}

// --------------------------------------------------------------------------------------------------------------------
// DEFINITION OF PRIVATE FUNCTIONS
// --------------------------------------------------------------------------------------------------------------------
static void helper_set_tx_isr_handler(std::function<void(void)> f)
{
    tx_isr_handler = f;
}

static void helper_set_rx_isr_handler(std::function<void(void)> f)
{
    rx_isr_handler = f;
}

static void tx_isr_handler_callback(int signal)
{
    tx_isr_handler();
    if (tx_isr_enabled)
        std::raise(SIGNAL_TX);
}

static void rx_isr_handler_callback(int signal)
{
    rx_isr_handler();
}

static void helper_set_byte_sender(std::function<void(char)> f)
{
    byte_sender = f;
}

static void tx_it_enable()
{
    tx_isr_enabled = true;
    std::raise(SIGNAL_TX);
}

static void tx_it_disable()
{
    tx_isr_enabled = false;
}

static void rx_it_enable()
{
}

static void rx_it_disable()
{
}

static void byte_send(char c)
{
    byte_sender(c);
}
