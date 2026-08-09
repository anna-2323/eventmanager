function modal({ id, title, body, confirmId,
    confirmLabel = "Запази",
    confirmClass = "is-link"
}) {
    return `
        <div class="modal" id="${id}">
            <div class="modal-background"></div>

            <div class="modal-card">
                <header class="modal-card-head">
                    <p class="modal-card-title">${title}</p>

                    <button class="delete modal-close"></button>
                </header>

                <section class="modal-card-body">
                    ${body}
                </section>

                <footer class="modal-card-foot">
                    <button
                        class="button ${confirmClass}"
                        id="${confirmId}">
                        ${confirmLabel}
                    </button>

                    <button class="button modal-cancel">
                        Отказ
                    </button>
                </footer>
            </div>
        </div>
    `;
}

function openModal(id) {
    $("#" + id).classList.add("is-active");
}

function closeModal(id) {
    $("#" + id).classList.remove("is-active");
}