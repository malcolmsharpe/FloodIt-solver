#include <bitset>
#include <cassert>
#include <cstdio>
#include <queue>
#include <set>
#include <string>
using namespace std;

#define FOR(i,n) for(int i=0;i<n;++i)

const int MAXR=16, MAXC=16;
const int NBITS=MAXR*MAXC;

int toidx(int r, int c) {
  return r*MAXC + c;
}

const int NCOLOURS = 6;
int fromcolour(char ch) {
  switch (ch) {
    case '.': return -1;
    case 'B': return 0;
    case 'G': return 1;
    case 'M': return 2;
    case 'O': return 3;
    case 'R': return 4;
    case 'Y': return 5;
  }
  assert(false);
  return -2;
}

char tocolour(int x) {
  switch (x) {
    case -1: return '.';
    case 0: return 'B';
    case 1: return 'G';
    case 2: return 'M';
    case 3: return 'O';
    case 4: return 'R';
    case 5: return 'Y';
  }
  assert(false);
  return 'x';
}

int R, C;
int board[MAXR][MAXC];

const int D = 4;
int dr[] = { 0, 1, 0, -1 };
int dc[] = { 1, 0, -1, 0 };

const int inf = 123456789;
int depth[MAXR][MAXC];
void compute_depth(const bitset<NBITS> &dead) {
  bool mark[MAXR][MAXC];
  memset(mark, 0, sizeof(mark));

  FOR(r,R) FOR(c,C) depth[r][c] = inf;

  static deque<pair<int,int> > q;
  q.clear();
  q.push_back(make_pair(0,0));
  depth[0][0] = 0;
  while (q.size()) {
    int r = q.front().first, c = q.front().second;
    q.pop_front();

    if (mark[r][c]) continue;
    mark[r][c] = 1;

    FOR(i,D) {
      int r2 = r+dr[i], c2 = c+dc[i];
      if (r2<0 || r2>=R || c2<0 || c2>=C) continue;
      
      int d = depth[r][c];
      int incr = !dead[toidx(r2,c2)] && board[r2][c2] != board[r][c];
      d += incr;

      if (d < depth[r2][c2]) {
        depth[r2][c2] = d;

        if (incr) q.push_back(make_pair(r2,c2));
        else q.push_front(make_pair(r2,c2));
      }
    }
  }
}

int old_heuristic_value(const bitset<NBITS> &dead) {
  compute_depth(dead);

  int maxdepth[NCOLOURS] = {};

  FOR(r,R) FOR(c,C) if (!dead[toidx(r,c)]) {
    int u = board[r][c];
    maxdepth[u] = max(maxdepth[u], depth[r][c]);
  }

  int m = 0;
  FOR(u,NCOLOURS) m = max(m, maxdepth[u]);

  int ans = 0;
  for (int d = 1; d <= m; ++d) {
    int c = 0;
    FOR(u,NCOLOURS) if (maxdepth[u] >= d) ++c;
    ans = max(ans, c+d-1);
  }
  assert(ans < inf);
  return ans;
}

int new_heuristic_value(const bitset<NBITS> &dead) {
  // killed: all dead squares in the simulation
  // mark: all squares either dead or in a depth1 list
  // depth1: for each colour, a list of its depth1 squares
  // nondepth1: for each colour, how many squares not at depth1
  // q: queue for flood fill (actually a stack)

  bool killed[MAXR][MAXC] = {}, mark[MAXR][MAXC] = {};
  int nondepth1[NCOLOURS] = {};
  static vector<pair<int,int> > depth1[NCOLOURS];
  FOR(u,NCOLOURS) depth1[u].clear();
  static vector<pair<int,int> > q;
  q.clear();

  // Initially, fill q with clear squares.
  FOR(r,R) FOR(c,C) {
    if (dead[toidx(r,c)]) {
      mark[r][c] = 1;
      q.push_back(make_pair(r,c));
    } else {
      ++nondepth1[board[r][c]];
    }
  }

  int ans = -1;
  while (1) {
    // If there is a colour with only depth-1 clumps, we will kill only that
    // colour.
    FOR(u,NCOLOURS) if (depth1[u].size() && nondepth1[u] == 0) {
      while (depth1[u].size()) {
        q.push_back(depth1[u].back());
        depth1[u].pop_back();
      }
      break;
    }

    // q is empty here if either this is not the first iteration (so the clear
    // squares aren't in q) and every colour has some clumps above depth 1.
    // In this case, we kill all clumps at depth 1.
    if (!q.size()) {
      FOR(u,NCOLOURS) {
        while (depth1[u].size()) {
          q.push_back(depth1[u].back());
          depth1[u].pop_back();
        }
      }

      if (!q.size()) break;
    }

    ++ans;

    // Kill every square in q. By doing this here, when propagating later in
    // the flood fill, we can recognize whether we're propagating from a
    // newly-killed square or a square of equal colour.
    FOR(i,(int)q.size()) {
      int r = q[i].first, c = q[i].second;
      killed[r][c] = 1;
    }

    while (q.size()) {
      int r = q.back().first, c = q.back().second;
      q.pop_back();

      FOR(i,D) {
        int r2 = r+dr[i], c2 = c+dc[i];
        if (r2<0 || r2>=R || c2<0 || c2>=C) continue;
        
        // We propagate to squares we haven't marked yet so that we only
        // update depth1 and nondepth1 once. Propagation occurs when either
        // we're propagating from a newly-killed square (so (r2,c2) is newly
        // bordering a clear square) or we're propagating from a square of the
        // same colour (so (r2,c2) is in the same clump as (r,c) and thus is
        // also now at depth 1).
        if (!mark[r2][c2] && (killed[r][c] || board[r2][c2] == board[r][c])) {
          mark[r2][c2] = 1;
          q.push_back(make_pair(r2,c2));

          depth1[board[r2][c2]].push_back(make_pair(r2,c2));
          --nondepth1[board[r2][c2]];
        }
      }
    }
  }

  return ans;
}

int heuristic_value(const bitset<NBITS> &dead) {
  return new_heuristic_value(dead);
}

void dump_depth() {
  printf("\n");
  FOR(r,R) {
    printf("[ ");
    FOR(c,C) {
      printf("%2d ", depth[r][c]);
    }
    printf("]\n");
  }
}

struct state {
  int moves;
  int heuristic;
  string path;
  vector<int> heur_path;
  int prev_seen_index;
  bitset<NBITS> dead;

  int key() const {
    return moves + heuristic;
  }
};

struct TrieNode {
  TrieNode *next[2];

  TrieNode() {
    memset(next, 0, sizeof(next));
  }
};

struct BitsTrie {
  TrieNode *root;

  BitsTrie() : root(new TrieNode) {}

  bool test_and_add(const bitset<NBITS> &s) {
    bool found = 1;
    TrieNode **pNode = &root;
    FOR(i,NBITS) {
      pNode = &(*pNode)->next[s[i]];

      if (*pNode == 0) {
        *pNode = new TrieNode;
        found = 0;
      }
    }
    return found;
  }
};

bool subseteq(const bitset<NBITS> &s, const bitset<NBITS> &t) {
  return (s & (~t)).none();
}

bool operator<(const state &a, const state &b) {
  if (a.key() != b.key()) return a.key() > b.key();

  // To break ties in the A* heap, observe that the subset optimization benefits
  // from having already visited many states with few gems, so that we are more
  // likely to prune.
  return a.dead.count() < b.dead.count();
}

struct PeterStats {
  int moves;
  int est1k, est1p5k, est2k;
  int loops;
  int old;
  string game_as_string;

  PeterStats()
    : moves(-1),
      est1k(-1), est1p5k(-1), est2k(-1),
      loops(-1), old(-1) {
  }

  void print() {
    const char *fn = "floodit_peter_stats.csv";

    FILE *f = fopen(fn, "r");
    bool stats_file_exists = bool(f);
    if (f) fclose(f);

    f = fopen(fn, "a");

    if (!stats_file_exists) {
      fprintf(f, "\"Moves\",\"Est1K\",\"Est1.5K\",\"Est2K\",\"Loops\",\"Old\","
                 "\"Game as String\"\n");
    }

    print_value(f, moves);
    print_value(f, est1k);
    print_value(f, est1p5k);
    print_value(f, est2k);
    print_value(f, loops);
    print_value(f, old);
    fprintf(f, "\"%s\"\n", game_as_string.c_str());

    fclose(f);
  }

  void print_value(FILE *f, int value) {
    if (value != -1) fprintf(f, "%d", value);
    fprintf(f, ",");
  }
};

void dump_dead(const bitset<NBITS> &dead) {
  FOR(r,R) {
    FOR(c,C) {
      char ch;
      if (dead[toidx(r,c)]) ch = '.';
      else ch = tocolour(board[r][c]);
      printf("%c", ch);
    }
    printf("\n");
  }
}

int main() {
  scanf(" %d%d",&R,&C);

  if (R > MAXR || C > MAXC) {
    fprintf(stderr, "Dimensions %d x %d exceed maximum!\n", R, C);
    exit(1);
  }

  bitset<NBITS> wanted;
  FOR(r,R) {
    FOR(c,C) {
      char ch;
      scanf(" %c", &ch);
      board[r][c] = fromcolour(ch);

      wanted[toidx(r,c)] = 1;
    }
  }

  state init;
  init.moves = 0;
  init.dead[toidx(0,0)] = 1;
  init.heuristic = heuristic_value(init.dead);
  init.prev_seen_index = -1;

  int prevkey = 0;
  int nstates = 0;

  int n_heur_incr = 0;
  int npushed = 0;
  PeterStats ps;

  FOR(r,R) FOR(c,C) {
    ps.game_as_string.push_back('0' + board[r][c] + 1);
  }

  BitsTrie trie;
  vector<pair<bitset<NBITS>, int> > mark;
  vector<state> seen;
  priority_queue<state> q;
  q.push(init);

  state nexts[NCOLOURS];
  while (q.size()) {
    state cur = q.top();
    q.pop();

    if (trie.test_and_add(cur.dead)) continue;

    bool marked = 0;
    FOR(i,(int)mark.size()) {
      if (subseteq(cur.dead, mark[i].first) && mark[i].second <= cur.moves) {
        marked = 1;
        break;
      }
    }
    if (marked) continue;

    mark.push_back(make_pair(cur.dead, cur.moves));
    seen.push_back(cur);

    ++nstates;

    if (nstates == 1000) ps.est1k = cur.key();
    if (nstates == 1500) ps.est1p5k = cur.key();
    if (nstates == 2000) ps.est2k = cur.key();

    if (cur.key() > prevkey) {
      prevkey = cur.key();
      printf("  Key = %2d, |q| = %d, nstates = %d\n", cur.key(), (int)q.size(),
	nstates);
    }

    if (cur.dead == wanted) {
      printf("Done. Searched %d states; heur incr %d/%d pushed.\n",
        nstates, n_heur_incr, npushed);
      printf("%d\n", cur.moves);
      string path = cur.path;
      vector<int> heur_path = cur.heur_path;
      FOR(i,(int)path.size()) {
        printf("%c", path[i]);
      }
      printf("\n");

      printf("\n");
      FOR(i,(int)heur_path.size()) {
        printf("%3d", heur_path[i]);
      }
      printf("\n");
      FOR(i,(int)path.size()) {
        printf("%3c", path[i]);
      }
      printf("\n");

      if (0) {
        // Print state history.
        state hist = cur;
        while (1) {
          printf("\n");

          int old_heur = old_heuristic_value(hist.dead);
          int new_heur = new_heuristic_value(hist.dead);

          printf("Old heuristic = %d\n", old_heur);
          printf("New heuristic = %d\n", new_heur);

          dump_dead(hist.dead);
          if (hist.prev_seen_index == -1) break;
          hist = seen[hist.prev_seen_index];
        }
      }

      // Peter stats.
      ps.moves = cur.moves;
      ps.loops = nstates;
      ps.print();

      return 0;
    }

    compute_depth(cur.dead);

    bool try_colour[NCOLOURS] = {};
    FOR(r,R) FOR(c,C) if (depth[r][c] == 1) {
      try_colour[board[r][c]] = 1;
    }

    FOR(u,NCOLOURS) if (try_colour[u]) {
      nexts[u] = cur;
      ++nexts[u].moves;
      nexts[u].path.push_back(tocolour(u));
      nexts[u].heur_path.push_back(cur.heuristic);
      nexts[u].prev_seen_index = int(seen.size()) - 1;

      FOR(r,R) FOR(c,C) if (depth[r][c] == 1 && board[r][c] == u) {
        nexts[u].dead[toidx(r,c)] = 1;
      }
    }

    FOR(u,NCOLOURS) if (try_colour[u]) {
      nexts[u].heuristic = heuristic_value(nexts[u].dead);
      ++npushed;
      if (nexts[u].key() == cur.key()+1) ++n_heur_incr;
      q.push(nexts[u]);
    }
  }

  printf("No solution found.\n");
}
