import React from 'react';
import { createRoot } from 'react-dom/client';

const container = document.getElementById('root');

if (!container)
    throw new Error('Container not found');

createRoot(container).render(
    <React.StrictMode>
        <h1>Hello!!</h1>
    </React.StrictMode>
);