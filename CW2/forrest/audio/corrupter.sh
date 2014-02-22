#!/bin/bash
 ./merge \
    <(./merge \
        <(cat -) \
        <(./merge \
            <(./signal_generator 500) \
            <(./signal_generator 600) \
        ) \
    ) \
    <(./merge \
        <(./merge \
            <(./signal_generator 800) \
            <(./signal_generator 1000) \
        ) \
        <(./merge \
            <(./signal_generator 1200) \
            <(./signal_generator 1400) \
        ) \
    ) \
