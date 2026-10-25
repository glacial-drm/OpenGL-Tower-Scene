/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "particle_generator.h"

ParticleGenerator::ParticleGenerator(GLuint program, unsigned int texture, unsigned int amount)
    : program(program), texture(texture), amount(amount)
{
    this->init();
}

void ParticleGenerator::Update(float dt, glm::vec3 position, unsigned int newParticles, glm::vec3 offset, glm::vec3 velocity)
{
    // add new particles 
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->firstUnusedParticle();
        this->respawnParticle(this->particles[unusedParticle], position, velocity, offset);
    }
    // update all particles
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        p.Life -= dt; // reduce life
        if (p.Life > 0.0f)
        {	// particle is alive, thus update
            p.Position -= p.Velocity * dt;
            p.Color.a -= dt * 2.5f;
        }
    }
}

// render all particles
void ParticleGenerator::Draw(glm::mat4 view, glm::mat4 projection, glm::mat4 model)
{
    
    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glUseProgram(this->program); // this->shader.Use();

    for (Particle particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {   
            glUniformMatrix4fv(glGetUniformLocation(this->program, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(this->program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(this->program, "model"), 1, GL_FALSE, glm::value_ptr(model));

            glUniform3f(glGetUniformLocation(this->program, "offset"), particle.Position.x, particle.Position.y, particle.Position.z); // this->shader.SetVector2f("offset", particle.Position);
            
            
            glUniform4f(glGetUniformLocation(this->program, "color"), particle.Color.x, particle.Color.y, particle.Color.z, particle.Color.w); // this->shader.SetVector4f("color", particle.Color);

            glBindTexture(GL_TEXTURE_2D, this->texture); // this->texture.Bind();
            
            glBindVertexArray(this->VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::init()
{
    // set up mesh and attribute properties
    unsigned int VBO;
    float particle_quad[] = {
        0.0f, 1.0f, 0.f,     0.0f, 1.0f,
        1.0f, 0.0f, 0.f,     1.0f, 0.0f,
        0.0f, 0.0f, 0.f,     0.0f, 0.0f,

        0.0f, 1.0f, 0.f,     0.0f, 1.0f,
        1.0f, 1.0f, 0.f,     1.0f, 1.0f,
        1.0f, 0.0f, 0.f,     1.0f, 0.0f
    };
    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);
    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // create this->amount default particle instances
    for (unsigned int i = 0; i < this->amount; ++i)
        this->particles.push_back(Particle());
}

// stores the index of the last particle used (for quick access to next dead particle)
unsigned int lastUsedParticle = 0;
unsigned int ParticleGenerator::firstUnusedParticle()
{
    // first search from last used particle, this will usually return almost instantly
    for (unsigned int i = lastUsedParticle; i < this->amount; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i) {
        if (this->particles[i].Life <= 0.0f) {
            lastUsedParticle = i;
            return i;
        }
    }
    // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
    lastUsedParticle = 0;
    return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, glm::vec3 position, glm::vec3 velocity, glm::vec3 offset) // re defined -----------------------------
{
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);
    particle.Position = position + random + offset;
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 1.0f;
    particle.Velocity = velocity * 0.1f;
}