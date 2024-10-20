#include <kernel/types.h>
#include <user/user.h>

#define NDIGRAPH 1000

#define MINVTX 100
#define MAXVTX 200

#define MINEDG 50
#define MAXEDG 400

#define REP(t) for (uint _ = t; _-- > 0; )

struct digraph_t {
  uint n;
  uint* adj;
};

struct node_t {
  struct node_t* next;
  uint val;
};

void
enqueue(struct node_t **head, struct node_t **tail, uint val)
{
  struct node_t *node = malloc(sizeof(struct node_t));

  node->next = 0;
  node->val = val;

  if (*head == 0) {
    *head = *tail = node;
  } else {
    *tail = ((*tail)->next = node);
  }
}

uint
pop(struct node_t **head)
{
  if (*head == 0) return 0;
  uint val = (*head)->val;
  struct node_t *node = (*head)->next;
  free(*head);
  *head = node;
  return val;
}

struct digraph_t
gendigraph()
{
  uint n = rng_range(MINVTX, MAXVTX);
  uint m = rng_range(MINEDG, MAXEDG);

  uint *adj = malloc(n * n * sizeof(uint));
  memset(adj, 0, n * n * sizeof(uint));

  REP(m) {
    uint u, v;

    do {
      u = rng_range(0, n - 1);
      v = rng_range(0, n - 1);
    } while (u == v || adj[n * u + v]);

    adj[n * u + v] = 1;
  }

  return (struct digraph_t) {
    .n = n,
    .adj = adj
  };
}

void
solve(struct digraph_t d)
{
  uint n = d.n;
  uint* dist = malloc(n * n * sizeof(uint));
  uint* adj = d.adj;

  memset(dist, 0xff, n * n * sizeof(uint));

  for (uint s = 0; s < n; s++) {
    struct node_t *head = 0;
    struct node_t *tail = 0;

    dist[n * s + s] = 0;
    enqueue(&head, &tail, s);
    while (head != 0) {
      uint u = pop(&head);

      for (uint v = 0; v < n; v++) {
        if (adj[n * u + v] && dist[n * s + v] < 0) {
          dist[n * s + v] = dist[n * s + u] + 1;
          enqueue(&head, &tail, v);
        }
      }
    }
  }

  free(dist);
}

void
digraphround()
{
  struct digraph_t d = gendigraph();
  solve(d);
  free(d.adj);
}

int
main(int argc, char** argv)
{
  REP(NDIGRAPH) digraphround();
  exit(0);
}
