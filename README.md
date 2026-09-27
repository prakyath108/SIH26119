# Sovereign Optimization Platform

A private AI-powered optimization platform for industrial operations, combining large language models with deterministic mathematical solvers.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                     User Access Layer                           │
│  Next.js Web UI  ◄──►  API Gateway  ◄──►  Identity Provider    │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    Application Layer                            │
│  ┌──────────┐ ┌─────────────┐ ┌────────────┐ ┌──────────────┐  │
│  │  Model   │ │  Scenario   │ │  Solver    │ │  AI          │  │
│  │ Service  │ │  Service    │ │  Scheduler │ │ Orchestrator │  │
│  └──────────┘ └─────────────┘ └────────────┘ └──────────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
              ┌───────────────┼───────────────┐
              ▼               ▼               ▼
       ┌──────────┐    ┌───────────┐   ┌────────────┐
       │PostgreSQL│    │  Redis    │   │  MinIO     │
       │+ pgvector│    │  (Cache/  │   │  (Objects) │
       │          │    │  Queue)   │   │            │
       └──────────┘    └───────────┘   └────────────┘
              │
              ▼
       ┌──────────────┐
       │  vLLM GPU    │
       │  Inference   │
       └──────────────┘
```

## Quick Start

### Prerequisites
- Docker & Docker Compose
- Node.js 20+ (for frontend development)
- Python 3.11+ (for backend development)
- NVIDIA GPU with 48GB+ VRAM (for LLM inference)

### Local Development

```bash
# Clone and enter
cd sovereign-platform

# Start all services
docker-compose up -d

# Check status
docker-compose ps

# View logs
docker-compose logs -f api
```

### Services
| Service | Port | Description |
|---------|------|-------------|
| API | 8000 | FastAPI backend |
| vLLM | 8001 | LLM inference |
| PostgreSQL | 5432 | Database |
| Redis | 6379 | Cache & Queue |
| MinIO | 9000/9001 | Object Storage |
| Web UI | 3000 | Next.js Frontend |

### API Documentation
- Swagger UI: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

## Project Structure

```
sovereign-platform/
├── apps/
│   ├── web/              # Next.js Frontend
│   └── api/              # FastAPI Backend
├── services/
│   ├── ai-orchestrator/  # LangGraph workflows
│   ├── model-generation/ # Typed model DSL
│   ├── knowledge/        # RAG & Document ingestion
│   ├── inference-gateway/# vLLM proxy
│   ├── solver-scheduler/ # Job queue
│   ├── solver-workers/   # CPU solver pool
│   └── audit/            # Audit logging
├── packages/
│   ├── api-contracts/    # OpenAPI schemas
│   ├── model-schema/     # Typed model DSL
│   ├── authz/            # Authorization
│   └── observability/    # Metrics/Traces
├── infrastructure/
│   ├── kubernetes/       # K8s manifests
│   ├── helm/             # Helm charts
│   ├── terraform/        # AWS infrastructure
│   ├── network-policies/ # Network isolation
│   └── monitoring/       # Prometheus/Grafana
├── models/
│   ├── registry-manifests/
│   ├── evaluation-datasets/
│   └── prompt-templates/
└── tests/
    ├── unit/
    ├── integration/
    ├── security/
    ├── model-evaluation/
    └── performance/
```

## Core Features

### 1. AI Copilot
- Natural language model generation
- Solver result explanations
- Scenario proposals
- Technical document Q&A

### 2. Model Studio
- Typed mathematical DSL
- Schema validation
- Dimensional analysis
- Version control & approvals

### 3. Solver Center
- LP/MILP/QP/MIQP solvers
- Job queue with priorities
- Real-time progress via WebSocket
- Result comparison & export

### 4. Knowledge Center
- Private document ingestion
- Access-controlled RAG
- Citation-backed responses
- Technical document search

### 5. AI Operations
- Model registry & versioning
- Inference monitoring
- GPU utilization tracking
- Evaluation benchmarks

## Development

### Backend
```bash
cd services/api
pip install -e .[dev]
uvicorn api.main:app --reload
```

### Frontend
```bash
cd apps/web
npm install
npm run dev
```

### Tests
```bash
# Backend
cd services/api
pytest -v

# Frontend
cd apps/web
npm test
```

## Deployment

### Kubernetes (Production)
```bash
# Apply namespace and config
kubectl apply -f infrastructure/kubernetes/namespace.yaml
kubectl apply -f infrastructure/kubernetes/configmap.yaml
kubectl apply -f infrastructure/kubernetes/secret.yaml

# Deploy services
kubectl apply -f infrastructure/kubernetes/postgres.yaml
kubectl apply -f infrastructure/kubernetes/redis.yaml
kubectl apply -f infrastructure/kubernetes/minio.yaml
kubectl apply -f infrastructure/kubernetes/vllm-deployment.yaml
kubectl apply -f infrastructure/kubernetes/api-deployment.yaml
kubectl apply -f infrastructure/kubernetes/worker-deployment.yaml
kubectl apply -f infrastructure/kubernetes/web-deployment.yaml
kubectl apply -f infrastructure/kubernetes/ingress.yaml
kubectl apply -f infrastructure/kubernetes/network-policies.yaml
```

### AWS (Terraform)
```bash
cd infrastructure/terraform
terraform init
terraform plan -var="environment=production"
terraform apply
```

## Security

- **Network Isolation**: Kubernetes NetworkPolicies enforce zero-trust
- **Encryption**: TLS 1.3 everywhere, AES-256 at rest
- **Authentication**: OIDC + MFA, short-lived JWT tokens
- **Authorization**: RBAC with row-level security
- **Audit**: Immutable append-only event log
- **Secrets**: HashiCorp Vault / AWS Secrets Manager
- **Supply Chain**: Signed images, pinned dependencies, SBOM

## Monitoring

- **Metrics**: Prometheus + Grafana
- **Tracing**: OpenTelemetry + Jaeger
- **Logging**: Structured JSON + Loki
- **Alerting**: PrometheusRule + Alertmanager

Key dashboards:
- API latency, error rate, throughput
- GPU utilization, inference latency, queue depth
- Solver queue depth, job duration, success rate
- Database connections, query latency, replication lag

## License

Proprietary - Internal Use Only