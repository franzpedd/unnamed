// enum for file types
const FileType = {
    C: 'C Source',
    H: 'C Header'
};

// all files
const fileData = [
    {
        id: 1,
        name: "buffer/cren_buffer.h",
        type: FileType.H,
        functions: [
            "buffer/BufferConstant",
            "buffer/BufferCamera",
            "buffer/BufferQuad"
        ]
    },

    {
        id: 2,
        name: "camera/cren_camera.h",
        type: FileType.H,
        functions: [
            "camera/cren_camera_create",
            "camera/cren_camera_destroy",
            "camera/cren_camera_update",
            "camera/cren_camera_get_aspect_ratio",
            "camera/cren_camera_set_aspect_ratio",
            "camera/cren_camera_get_fov",
            "camera/cren_camera_translate",
            "camera/cren_camera_rotate",
            "camera/cren_camera_get_view",
            "camera/cren_camera_get_view_inverse",
            "camera/cren_camera_get_perspective",
            "camera/cren_camera_get_perspective_inverse",
            "camera/cren_camera_get_lock",
            "camera/cren_camera_set_lock",
            "camera/cren_camera_move",
            "camera/cren_camera_get_speed_modifier",
            "camera/cren_camera_set_speed_modifier",
            "camera/cren_camera_get_position",
            "camera/cren_camera_get_front"
        ]
    },

    {
        id: 3,
        name: "context/cren_context.h",
        type: FileType.H,
        functions: [
            "context/CRenCreateInfo",
            "context/cren_initialize",
            "context/cren_shutdown",
            "context/cren_create_renderer",
            "context/cren_update",
            "context/cren_render",
            "context/cren_resize",
            "context/cren_minimize",
            "context/cren_restore",
            "context/cren_get_main_camera",
            "context/cren_are_validations_enabled",
            "context/cren_using_vsync",
            "context/cren_get_msaa",
            "context/cren_using_custom_viewport",
            "context/cren_get_vulkan_backend",
            "context/cren_get_mousepos",
            "context/cren_set_mousepos",
            "context/cren_get_viewport_pos",
            "context/cren_set_viewport_pos",
            "context/cren_get_viewport_size",
            "context/cren_set_viewport_size",
            "context/cren_get_framebuffer_size",
            "context/cren_set_framebuffer_size",
            "context/cren_pick_object",
            "context/cren_create_id",
            "context/cren_register_id",
            "context/cren_unregister_id",
            "context/cren_set_user_pointer",
            "context/cren_get_user_pointer",
            "context/cren_set_render_callback",
            "context/cren_set_resize_callback",
            "context/cren_set_ui_image_count_callback",
            "context/cren_set_draw_ui_raw_data_callback",
            "context/cren_set_get_vulkan_instance_required_extensions_callback",
            "context/cren_set_create_vulkan_surface_callback"
        ]
    },

    {
        id: 4,
        name: "defines/cren_defines.h",
        type: FileType.H,
        functions: [
            "defines/align_as",
            "defines/CREN_API"
        ]
    },

    {
        id: 5,
        name: "error/cren_error.h",
        type: FileType.H,
        functions: [
            "error/cren_log_message"
        ]
    },

    {
        id: 6,
        name: "platform/cren_platform.h",
        type: FileType.H,
        functions: [
            "platform/cren_detect_platform",
            "platform/cren_get_path",
            "platform/cren_stbimage_load_from_file",
            "platform/cren_stbimage_destroy",
            "platform/cren_stbimage_get_error",
            "platform/cren_load_file"
        ]
    },

    {
        id: 7,
        name: "primitives/cren_primitives.h",
        type: FileType.H,
        functions: [
            "primitives/cren_texture2d_create_from_path",
            "primitives/cren_texture2d_create_from_buffer",
            "primitives/cren_texture2d_destroy",
            "primitives/cren_quad_create",
            "primitives/cren_quad_destroy",
            "primitives/cren_quad_update",
            "primitives/cren_quad_render",
            "primitives/cren_quad_get_id",
            "primitives/cren_quad_get_billboard",
            "primitives/cren_quad_get_lock_axis_x",
            "primitives/cren_quad_set_lock_axis_x",
            "primitives/cren_quad_get_lock_axis_y",
            "primitives/cren_quad_set_lock_axis_y"
        ]
    },

    {
        id: 8,
        name: "types/cren_types.h",
        type: FileType.H,
        functions: [
            "types/CRen_Platform",
            "types/CRen_CameraType",
            "types/CRen_CameraDirection",
            "types/CRen_RendererAPI",
            "types/CRen_MSAA",
            "types/CRen_ShaderType",
            "types/CRen_VertexComponent",
            "types/CRen_RenderStage"
        ]
    }
];

// removes the everything before first / of the string
function removeNamespace(name) {
    return name.includes('/') ? name.substring(name.lastIndexOf('/') + 1) : name;
}

// returns the namespace
function getNamespace(name) {
    return name.includes('/') ? name.substring(0, name.indexOf('/')) : null;
}

// finds the file by it's name
function getFileByName(name) {
    return fileData.find(file => file.name === name);
}

// finds the function by it's name
function getFunctionByName(funcName) {
    for (const file of fileData) {
        if (file.functions.includes(funcName)) {
            return {
                name: funcName,
                displayName: removeNamespace(funcName),
                file: file.name,
                fileDisplayName: removeNamespace(file.name)
            };
        }
    }
    return null;
}

// marks item as active
function setActiveItem(element) {
    document.querySelectorAll('.file-header, .function-item').forEach(item => {
        item.classList.remove('active');
    });
    element.classList.add('active');
}

// toggle file visibility 
function toggleFileFunctions(fileId) {
    const functionList = document.getElementById(`functions-${fileId}`);
    const icon = document.querySelector(`[data-file-id="${fileId}"] i`);
    if (functionList) functionList.classList.toggle('show');
    if (icon) icon.classList.toggle('icon-rotate');
}

// shows the explorer 
function displayExplorer() {
    const explorer = document.getElementById('explorer');
    if (!explorer) return;
    
    explorer.innerHTML = '';
    
    fileData.forEach(file => {
        const fileHeader = document.createElement('div');
        fileHeader.className = 'file-header';
        fileHeader.dataset.fileId = file.id;
        fileHeader.dataset.fullName = file.name;
        
        const displayName = removeNamespace(file.name);
        fileHeader.innerHTML = `
            <i class="fas fa-chevron-right"></i>
            <span class="file-name">${displayName}</span>
        `;
        
        fileHeader.onclick = () => {
            toggleFileFunctions(file.id);
            setActiveItem(fileHeader);
            displayFileContent(file.name);
        };
        
        explorer.appendChild(fileHeader);
        
        const functionList = document.createElement('div');
        functionList.className = 'function-list';
        functionList.id = `functions-${file.id}`;
        
        file.functions.forEach(funcName => {
            const funcItem = document.createElement('div');
            funcItem.className = 'function-item';
            
            const displayName = removeNamespace(funcName);
            funcItem.textContent = displayName;
            
            funcItem.dataset.fullName = funcName;
            
            funcItem.onclick = (e) => {
                e.stopPropagation();
                setActiveItem(funcItem);
                displayFunctionContent(funcName);
            };
            functionList.appendChild(funcItem);
        });
        
        explorer.appendChild(functionList);
    });
}

// displays the file's data
function displayFileContent(fileName) {
    const contentDisplay = document.getElementById('contentDisplay');
    if (!contentDisplay) return;
    
    console.log(`Loading file: docs/${fileName}.html`); // Debug
    
    fetch(`docs/${fileName}.html`)
        .then(response => {
            console.log('Response status:', response.status); // Debug
            if (response.ok) {
                return response.text();
            }
            throw new Error(`HTTP ${response.status}`);
        })
        .then(html => {
            contentDisplay.innerHTML = html;
        })
        .catch(error => {
            console.error('Error loading file:', error); // Debug
            const file = getFileByName(fileName);
            if (!file) {
                contentDisplay.innerHTML = `<h2>File not found: ${fileName}</h2>`;
                return;
            }
            
            const displayName = removeNamespace(file.name);
            const functionsDisplay = file.functions.map(f => removeNamespace(f)).join(', ');
            
            contentDisplay.innerHTML = `
                <h2>${displayName}</h2>
                <p><strong>Full Name:</strong> ${file.name}</p>
                <p><strong>Type:</strong> ${file.type}</p>
                <p><strong>Functions:</strong> ${functionsDisplay}</p>
                <div class="warning">
                    <i class="fas fa-exclamation-triangle"></i>
                    Documentation not found. Error: ${error.message}
                </div>
                <div class="note">
                    Create <code>api/docs/${fileName}.html</code> to add documentation.
                </div>
            `;
        });
}

// display the function's data
function displayFunctionContent(funcName) {
    const contentDisplay = document.getElementById('contentDisplay');
    if (!contentDisplay) return;
    
    const func = getFunctionByName(funcName);
    if (!func) {
        contentDisplay.innerHTML = `<h2>Function not found: ${funcName}</h2>`;
        return;
    }
    
    console.log(`Loading function: docs/${funcName}.html`); // Debug
    
    const file = getFileByName(func.file);
    if (file) {
        const fileHeader = document.querySelector(`[data-file-id="${file.id}"]`);
        const functionList = document.getElementById(`functions-${file.id}`);
        
        if (fileHeader && functionList && !functionList.classList.contains('show')) {
            toggleFileFunctions(file.id);
        }
    }
    
    fetch(`docs/${funcName}.html`)
        .then(response => {
            console.log('Response status:', response.status); // Debug
            if (response.ok) {
                return response.text();
            }
            throw new Error(`HTTP ${response.status}`);
        })
        .then(html => {
            contentDisplay.innerHTML = html;
        })
        .catch(error => {
            console.error('Error loading function:', error); // Debug
            contentDisplay.innerHTML = `
                <h2>${func.displayName}</h2>
                <p><strong>Full Name:</strong> ${func.name}</p>
                <p><strong>File:</strong> ${func.fileDisplayName}</p>
                <div class="warning">
                    <i class="fas fa-exclamation-triangle"></i>
                    Documentation not found. Error: ${error.message}
                </div>
                <div class="note">
                    Create <code>api/docs/${funcName}.html</code> to add documentation.
                </div>
            `;
        });
}

// search box
function setupSearch() {
    const searchInput = document.getElementById('searchInput');
    if (!searchInput) return;
    
    searchInput.addEventListener('input', function() {
        const searchTerm = this.value.toLowerCase().trim();
        
        if (searchTerm === '') {
            document.querySelectorAll('.file-header, .function-item').forEach(item => {
                item.style.display = 'flex';
            });
            return;
        }
        
        fileData.forEach(file => {
            const fileHeader = document.querySelector(`[data-file-id="${file.id}"]`);
            const functionList = document.getElementById(`functions-${file.id}`);
            if (!fileHeader || !functionList) return;
            
            const functions = functionList.querySelectorAll('.function-item');
            
            const fileDisplayName = removeNamespace(file.name).toLowerCase();
            const fileName = file.name.toLowerCase();
            const fileMatches = fileDisplayName.includes(searchTerm) || fileName.includes(searchTerm);
            
            let anyFunctionMatches = false;
            
            functions.forEach(funcItem => {
                const displayName = funcItem.textContent.toLowerCase();
                const fullName = funcItem.dataset.fullName.toLowerCase();
                
                // Search in both display name and full name
                const funcMatches = displayName.includes(searchTerm) || fullName.includes(searchTerm);
                
                funcItem.style.display = funcMatches ? 'flex' : 'none';
                if (funcMatches) anyFunctionMatches = true;
            });
            
            if (fileMatches || anyFunctionMatches) {
                fileHeader.style.display = 'flex';
                if (!functionList.classList.contains('show')) {
                    toggleFileFunctions(file.id);
                }
            } else {
                fileHeader.style.display = 'none';
            }
        });
    });
}

// entrypoint
function init() {
    displayExplorer();
    setupSearch();
    
    if (fileData.length > 0) {
        const firstFile = document.querySelector('.file-header');
        if (firstFile) {
            setActiveItem(firstFile);
            displayFileContent(fileData[0].name);
            toggleFileFunctions(fileData[0].id);
        }
    }
}

document.addEventListener('DOMContentLoaded', init);