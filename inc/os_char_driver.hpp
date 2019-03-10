/**
 * @file	os_char_driver.hpp
 * @brief	Defines a class which is used to handle reading/writing from e.g. UART port
 * @author	Kacper Kowalski - kacper.s.kowalski@gmail.com
 */

#ifndef OS_CHAR_DRIVER_HPP
#define OS_CHAR_DRIVER_HPP

#include "ibytestream_ostringstream.hpp"
#include "os.h"
#include "os_lockguard.hpp"
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

namespace jungles {

// --------------------------------------------------------------------------------------------------------------------
// DECLARATIONS AND DEFINITIONS FOR PUBLIC USE
// --------------------------------------------------------------------------------------------------------------------

/**
 * \brief Works similarly to linux's char driver. Allows to readlines and write strings to a device.
 *
 * This implementation assumes that it works on top of ISR handlers. Because of that RX and TX enablers/disablers must
 * be provided and also a function which allows to send a byte over the TX line. It is a wrapper over
 * jungles::ibytestream_ostringstream object used to received data (see it's documentation for details).
 * This class performs also a blocking write to make it possibly most memory effective - the strings passed to the
 * write() function are not copied to any internal buffer - they'ra streamed from the caller side.
 * The tx_isr_handler and rx_isr_handler must be called from ISR to make this class work as expected.
 */
template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf> class os_char_driver
{
  public:
    using PtrToVoidFunTakingVoid = void (*)(void);
    using PtrToVoidFunTakingChar = void (*)(char);

    explicit os_char_driver(PtrToVoidFunTakingVoid tx_it_enabler,
                            PtrToVoidFunTakingVoid tx_it_disabler,
                            PtrToVoidFunTakingVoid rx_it_enabler,
                            PtrToVoidFunTakingVoid rx_it_disabler,
                            PtrToVoidFunTakingChar byte_sender,
                            std::string_view rx_exceptional_chars = std::string_view{""},
                            std::string_view rx_string_terminators = std::string_view{"\0\r\n", 3});

    ~os_char_driver();

    /**
     * \brief Blocking write multiple strings.
     * \param[in] Can be vectors, arrays, string_views ... - underlying char array must occupy contiguous memory.
     */
    template <typename... StringTypes> void write(StringTypes &&... strings);
    std::string readline(unsigned timeout_ms);

    void tx_isr_handler();
    void rx_isr_handler(char c);

    // ----------------------------------------------------------------------------------------------------------------
    // DECLARATIONS AND DEFINITIONS FOR PRIVATE USE
    // ----------------------------------------------------------------------------------------------------------------
  private:
    struct events
    {
        static inline constexpr unsigned tx_end = 1;
    };

    const PtrToVoidFunTakingVoid m_tx_it_enabler;
    const PtrToVoidFunTakingVoid m_tx_it_disabler;
    const PtrToVoidFunTakingVoid m_rx_it_enabler;
    const PtrToVoidFunTakingVoid m_rx_it_disabler;
    const PtrToVoidFunTakingChar m_byte_sender;

    ibytestream_ostringstream<InternalRxBufSize, MaxNumStringsInRxBuf> m_rx_stream;
    os_counting_semaphore_t m_rx_msgs_counting_sem;
    os_mutex_t m_mux;
    os_event_group_handle_t m_events;
    std::pair<const char *, const char *> transmitted_string;

    template <typename StringType> void write_single_string(StringType &&string);
};

// --------------------------------------------------------------------------------------------------------------------
// PUBLIC MEMBERS' DEFINITIONS
// --------------------------------------------------------------------------------------------------------------------
template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::os_char_driver(PtrToVoidFunTakingVoid tx_it_enabler,
                                                                        PtrToVoidFunTakingVoid tx_it_disabler,
                                                                        PtrToVoidFunTakingVoid rx_it_enabler,
                                                                        PtrToVoidFunTakingVoid rx_it_disabler,
                                                                        PtrToVoidFunTakingChar byte_sender,
                                                                        std::string_view rx_exceptional_chars,
                                                                        std::string_view rx_string_terminators)
    : m_tx_it_enabler{tx_it_enabler}, m_tx_it_disabler{tx_it_disabler}, m_rx_it_enabler{rx_it_enabler},
      m_rx_it_disabler{rx_it_disabler}, m_byte_sender{byte_sender}, m_rx_stream{rx_exceptional_chars,
                                                                                rx_string_terminators},
      m_rx_msgs_counting_sem{os_counting_semaphore_create(MaxNumStringsInRxBuf * 2, 0)}, m_mux{os_mutex_create()},
      m_events{os_event_group_create()}
{
    (*m_rx_it_enabler)();
}

template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::~os_char_driver()
{
    (*m_rx_it_disabler)();
    (*m_tx_it_disabler)();
    os_counting_semaphore_delete(m_rx_msgs_counting_sem);
    os_mutex_delete(m_mux);
    os_event_group_delete(m_events);
}

template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
template <typename... StringTypes>
void os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::write(StringTypes &&... strings)
{
    os_lockguard g{m_mux};
    (write_single_string(strings), ...);
}

template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
std::string os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::readline(unsigned timeout_ms)
{
    os_lockguard g{m_mux};
    auto tmt = os_timeout_to_ticks(timeout_ms);
    if (os_counting_semaphore_take(m_rx_msgs_counting_sem, tmt) == os_true)
        return m_rx_stream.pop_string();
    else
        return "";
}

template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
void os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::tx_isr_handler()
{
    auto &[beg, end] = transmitted_string;
    if (beg == end)
    {
        os_event_group_set_bits_from_isr(m_events, events::tx_end);
        (*m_tx_it_disabler)();
    }
    else
    {
        (*m_byte_sender)(*beg++);
    }
}

template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
void os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::rx_isr_handler(char c)
{
    if (m_rx_stream.push_byte_and_is_string_end(c))
        os_counting_semaphore_give_from_isr(m_rx_msgs_counting_sem);
}

// --------------------------------------------------------------------------------------------------------------------
// PRIVATE MEMBERS' DEFINITIONS
// --------------------------------------------------------------------------------------------------------------------
template <size_t InternalRxBufSize, size_t MaxNumStringsInRxBuf>
template <typename StringType>
void os_char_driver<InternalRxBufSize, MaxNumStringsInRxBuf>::write_single_string(StringType &&string)
{
    auto &[beg, end] = transmitted_string;
    beg = &(*std::begin(string));
    end = beg + std::distance(std::begin(string), std::end(string));
    (*m_tx_it_enabler)();
    os_event_group_wait_bits_endlessly(m_events, events::tx_end, os_true, os_true);
}

} // namespace jungles

#endif /* OS_CHAR_DRIVER_HPP */
