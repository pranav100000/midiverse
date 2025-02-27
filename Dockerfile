FROM ubuntu:22.04 AS build

# Set noninteractive mode for apt
ENV DEBIAN_FRONTEND=noninteractive

# Install required packages
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libasound2-dev \
    libjack-jackd2-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libfreetype6-dev \
    libcurl4-openssl-dev \
    libasio-dev \
    python3 \
    python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create app directory
WORKDIR /app

# Copy the project files
COPY . .

# Install dependencies and make scripts executable
RUN chmod +x install_deps.sh && ./install_deps.sh

# Build just the CLI tool (without JUCE for smaller image, can be changed)
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make midiverse_cli -j$(nproc)

# Python dependencies
RUN pip3 install -r examples/requirements.txt

# Create a second stage for a smaller runtime image
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libasound2 \
    libjack-jackd2-0 \
    libcurl4 \
    python3 \
    python3-pip \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Create directories
WORKDIR /app
RUN mkdir -p output

# Copy built binaries and scripts from the build stage
COPY --from=build /app/build/midiverse_cli /app/build/
COPY --from=build /app/midiverse.py /app/
COPY --from=build /app/examples /app/examples

# Install Python dependencies
COPY --from=build /app/examples/requirements.txt /app/examples/
RUN pip3 install -r examples/requirements.txt

# Make scripts executable
RUN chmod +x /app/midiverse.py

# Create a volume for input/output files
VOLUME ["/app/input", "/app/output"]

# Set entrypoint to the Python wrapper
ENTRYPOINT ["python3", "/app/midiverse.py"]
CMD ["--help"]