# Medical Cabinet System

A multi-threaded medical cabinet management system using semaphores for thread synchronization.

## Overview

This project implements a medical cabinet system that handles concurrent access to medications using thread synchronization mechanisms. It ensures safe and controlled access to medical resources through semaphores.

## Features

- Thread-safe medication dispensing
- Concurrent access management
- Resource locking using semaphores
- Real-time inventory tracking

## Requirements

- C compiler (gcc recommended)
- POSIX-compliant system
- pthread library

## Building

```bash
gcc -o medical_cabinet main.c -pthread
```

## Usage

```bash
./medical_cabinet
```