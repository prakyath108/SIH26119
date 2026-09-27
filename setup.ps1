<# 
.SYNOPSIS
    Sovereign Optimization Platform - Automated Setup Script
.DESCRIPTION
    Sets up the complete development environment:
    - Starts Docker Desktop (if not running)
    - Fixes pyproject.toml (langgraph version)
    - Installs Python dependencies
    - Applies database schema
    - Starts the API server
#>

param(
    [switch]$SkipDockerCheck,
    [switch]$SkipDeps,
    [switch]$SkipDB,
    [switch]$NoAPI
)

$ErrorActionPreference = "Stop"
$projectRoot = "C:\Users\prakyath\sovereign-platform"
$apiDir = "$projectRoot\services\api"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Sovereign Platform - Automated Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# Function to check if Docker Desktop is running
function Check-Docker {
    Write-Host "`n[1/6] Checking Docker Desktop..." -ForegroundColor Yellow
    try {
        $dockerStatus = docker version --format '{{.Server.Version}}' 2>$null
        if ($dockerStatus) {
            Write-Host "  Docker is running (version: $dockerStatus)" -ForegroundColor Green
            return $true
        }
    } catch {}
    
    Write-Host "  Docker Desktop not running. Starting..." -ForegroundColor Yellow
    Start-Process "C:\Program Files\Docker\Docker\Docker Desktop.exe" -ErrorAction SilentlyContinue
    
    Write-Host "  Waiting for Docker Desktop to start (max 60s)..." -ForegroundColor Yellow
    $timeout = 60
    $start = Get-Date
    while (((Get-Date) - $start).TotalSeconds -lt $timeout) {
        try {
            $status = docker version --format '{{.Server.Version}}' 2>$null
            if ($status) {
                Write-Host "  Docker started (version: $status)" -ForegroundColor Green
                return $true
            }
        } catch {}
        Start-Sleep -Seconds 3
        Write-Host -NoNewline "."
    }
    Write-Host "`n  ERROR: Docker Desktop failed to start within 60s" -ForegroundColor Red
    Write-Host "  Please start Docker Desktop manually from Start Menu" -ForegroundColor Red
    return $false
}

# Function to fix pyproject.toml
function Fix-PyProject {
    Write-Host "`n[2/6] Fixing pyproject.toml (langgraph version)..." -ForegroundColor Yellow
    $pyprojectPath = "$apiDir\pyproject.toml"
    if (Test-Path $pyprojectPath) {
        $content = Get-Content $pyprojectPath -Raw
        if ($content -match 'langgraph==0\.1\.0') {
            $content = $content -replace 'langgraph==0\.1\.0', 'langgraph==0.2.0'
            Set-Content $pyprojectPath -Value $content
            Write-Host "  Fixed langgraph version to 0.2.0" -ForegroundColor Green
        } else {
            Write-Host "  Already fixed or version not found" -ForegroundColor Yellow
        }
    } else {
        Write-Host "  ERROR: pyproject.toml not found at $pyprojectPath" -ForegroundColor Red
        return $false
    }
    return $true
}

# Function to delete corrupted .env
function Fix-EnvFile {
    Write-Host "`n[3/6] Checking .env file..." -ForegroundColor Yellow
    $envPath = "$projectRoot\.env"
    if (Test-Path $envPath) {
        try {
            $content = Get-Content $envPath -Raw -ErrorAction Stop
            if ($content -match '[\x00-\x08\x0B\x0C\x0E-\x1F]') {
                Write-Host "  Corrupted .env detected, removing..." -ForegroundColor Yellow
                Remove-Item $envPath -Force
                Write-Host "  Removed corrupted .env" -ForegroundColor Green
            } else {
                Write-Host "  .env file OK" -ForegroundColor Green
            }
        } catch {
            Write-Host "  Could not read .env, removing..." -ForegroundColor Yellow
            Remove-Item $envPath -Force -ErrorAction SilentlyContinue
        }
    } else {
        Write-Host "  No .env file (will be created by docker-compose)" -ForegroundColor Green
    }
}

# Function to fix docker-compose.yml (remove container_name from worker)
function Fix-DockerCompose {
    Write-Host "`n[4/6] Fixing docker-compose.yml..." -ForegroundColor Yellow
    $composePath = "$projectRoot\docker-compose.yml"
    if (Test-Path $composePath) {
        $content = Get-Content $composePath -Raw
        if ($content -match 'container_name:\s*sovereign-worker') {
            $content = $content -replace 'container_name:\s*sovereign-worker\s*\r?\n', ''
            Set-Content $composePath -Value $content
            Write-Host "  Removed container_name from worker service" -ForegroundColor Green
        } else {
            Write-Host "  Already fixed" -ForegroundColor Yellow
        }
    }
}

# Function to install Python dependencies
function Install-Deps {
    if ($SkipDeps) { return $true }
    Write-Host "`n[5/6] Installing Python dependencies..." -ForegroundColor Yellow
    Set-Location $apiDir
    
    # Ensure venv exists
    if (-not (Test-Path "venv")) {
        Write-Host "  Creating virtual environment..." -ForegroundColor Yellow
        python -m venv venv
    }
    
    # Activate and install
    & ".\venv\Scripts\Activate.ps1"
    
    Write-Host "  Installing langgraph 0.2.0..." -ForegroundColor Yellow
    pip install langgraph==0.2.0
    
    Write-Host "  Installing package in development mode..." -ForegroundColor Yellow
    pip install -e .[dev]
    
    Write-Host "  Dependencies installed" -ForegroundColor Green
    return $true
}

# Function to apply database schema
function Apply-DBSchema {
    if ($SkipDB) { return $true }
    Write-Host "`n[6/6] Applying database schema..." -ForegroundColor Yellow
    Set-Location $projectRoot
    
    # Check if postgres container is running
    $container = docker ps --filter "name=sovereign-platform-postgres" --format "{{.Names}}" 2>$null
    if (-not $container) {
        Write-Host "  Starting PostgreSQL container..." -ForegroundColor Yellow
        docker-compose up -d postgres
        Start-Sleep -Seconds 5
    }
    
    Write-Host "  Applying schema..." -ForegroundColor Yellow
    $schemaPath = "$projectRoot\infrastructure\database\001_init_schema.sql"
    if (Test-Path $schemaPath) {
        Get-Content $schemaPath | docker exec -i sovereign-platform-postgres-1 psql -U sovereign -d sovereign
        Write-Host "  Database schema applied" -ForegroundColor Green
    } else {
        Write-Host "  ERROR: Schema file not found" -ForegroundColor Red
        return $false
    }
    return $true
}

# Function to start API
function Start-API {
    if ($NoAPI) { return $true }
    Write-Host "`n[7/7] Starting API server..." -ForegroundColor Yellow
    Set-Location $apiDir
    & ".\venv\Scripts\Activate.ps1"
    $env:PYTHONPATH = "src"
    Write-Host "Starting API on http://localhost:8000" -ForegroundColor Cyan
    Write-Host "Press Ctrl+C to stop" -ForegroundColor Yellow
    uvicorn api.main:app --host 0.0.0.0 --port 8000 --reload
}

# ==========================================
# MAIN EXECUTION
# ==========================================

try {
    Write-Host "Starting setup at $(Get-Date)" -ForegroundColor Cyan
    
    # Check admin rights
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
    if (-not $isAdmin) {
        Write-Host "WARNING: Not running as Administrator. Docker commands may fail." -ForegroundColor Yellow
    }
    
    # Run all steps
    if (-not $SkipDockerCheck) { if (-not (Check-Docker)) { throw "Docker not available" } }
    Fix-EnvFile
    Fix-DockerCompose
    if (-not (Fix-PyProject)) { throw "Failed to fix pyproject.toml" }
    if (-not (Install-Deps)) { throw "Failed to install dependencies" }
    if (-not (Apply-DBSchema)) { throw "Failed to apply database schema" }
    
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host "SETUP COMPLETE!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host "API will start on: http://localhost:8000" -ForegroundColor Cyan
    Write-Host "API Docs: http://localhost:8000/docs" -ForegroundColor Cyan
    Write-Host "Press Ctrl+C to stop the server" -ForegroundColor Yellow
    Write-Host "========================================`n" -ForegroundColor Cyan
    
    if (-not $NoAPI) {
        Start-API
    }
    
} catch {
    Write-Host "`nERROR: $($_.Exception.Message)" -ForegroundColor Red
    Write-Host "Setup failed. Check the error above." -ForegroundColor Red
    exit 1
}