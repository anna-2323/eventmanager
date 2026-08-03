const BASE_URL = '/api';

async function request(path = {}, options = {}) {
  const res = await fetch(BASE_URL + path, {
    headers: { 'Content-Type': 'application/json', ...options.headers },
    ...options,
    credentials: 'include',
  });
  if (!res.ok) throw new Error(`API error ${res.status}: ${await res.text()}`);
  return res.status === 204 ? null : res.json();
}

export const api = {
  auth: {
    getUser: () => request('/me'),
    login: (data) => request('/login', { method: 'POST', body: JSON.stringify(data) }),
    logout: () => request('/logout', { method: 'POST' }),
    signup: (data) => request('/signup', { method: 'POST', body: JSON.stringify(data) }),
    forgotPassword: (email) => request('/forgot', { method: 'POST', body: JSON.stringify({ email }) }),
    resetPassword: (data) => request('/reset', { method: 'POST', body: JSON.stringify(data) }),
  },

  events: {
    list: (params) => request('/events' + toQuery(params)),
    get: (id) => request(`/events/${id}`),
    getSeatMap: (id) => request(`/events/layout/${id}`),
  },

  tickets: {
    purchase: (id, data) => request('/purchase/' + id, { method: 'POST', body: JSON.stringify(data) }),
    confirm: (token) => request(`/confirmation/${token}`),
  },

  users: {
    // getProfile: () => request('/profile'),
    updateProfile: (data, path) => request('/profile' + path, { method: 'PATCH', body: JSON.stringify(data) }),
    deleteProfile: (data) => request('/profile/delete', { method: 'DELETE', body: JSON.stringify(data) })
  },

  admin: {
    users: {
      list: () => request('/users'),
    //  delete: (id) => request(`/admin/users/${id}`, { method: 'DELETE' })
    },
    events: {
      list: () => request('/events'),
    //   create: (data) => request('/admin/events', { method: 'POST', body: JSON.stringify(data) }),
    //   update: (id, data) => request(`/admin/events/${id}`, { method: 'PUT', body: JSON.stringify(data) }),
    //   delete: (id) => request(`/admin/events/${id}`, { method: 'DELETE' }),
    },
  },
};

function toQuery(params) {
  if (!params) return '';
  return '?' + new URLSearchParams(params).toString();
}
