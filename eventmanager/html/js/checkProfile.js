export function checkProfile() {
  fetch("/api/me")
    .then((r) => r.json())
    .then((user) => {
      const navEnd = document.querySelector(".navbar-end");
      if (user.logged_in) {
        document.getElementById("login-btn").outerHTML = `
                <div class="navbar-item has-dropdown is-hoverable">
                    <a class="navbar-link">
                        <span class="icon"><i class="fas fa-user"></i></span>
                        <span>${user.email}</span>
                    </a>
                    <div class="navbar-dropdown is-right">
                        <a class="navbar-item" href="/profile">Профил</a>
                        <hr class="navbar-divider">
                        <a class="navbar-item" id="logout-btn">Изход</a>
                    </div>
                </div>
            `;
        document.getElementById("logout-btn").addEventListener("click", () => {
          fetch("/api/logout", { method: "POST" }).then(() =>
            window.location.reload(),
          );
        });
      }
    });
}
